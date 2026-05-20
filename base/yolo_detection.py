import os
import warnings
os.environ["ONNX_ML"] = "0"
os.environ["TF_CPP_MIN_LOG_LEVEL"] = "3"
warnings.filterwarnings("ignore")

import cv2
import serial
import time
import requests
import numpy as np
from ultralytics import YOLO

ESP32_STREAM_URL = "http://192.168.1.11/stream"
SERIAL_PORT      = "/dev/ttyUSB0"
BAUD_RATE        = 9600

FRAME_SKIP    = 3
SEND_INTERVAL = 5

DETECTION_LABELS = {
    "person":   "P",
    "multiple": "M",
    "clear":    "C",
    "unknown":  "U"
}

def get_detection_code(results):
    person_count = 0
    has_other = False
    for result in results:
        for box in result.boxes:
            label = result.names[int(box.cls)]
            if label == "person":
                person_count += 1
            else:
                has_other = True
    if person_count > 1:
        return DETECTION_LABELS["multiple"]
    elif person_count == 1:
        return DETECTION_LABELS["person"]
    elif has_other:
        return DETECTION_LABELS["unknown"]
    else:
        return DETECTION_LABELS["clear"]

def get_frames(url):
    """Generator that yields frames from MJPEG stream using requests."""
    stream = requests.get(url, stream=True, timeout=30)
    buffer = bytes()
    for chunk in stream.iter_content(chunk_size=4096):
        buffer += chunk
        start = buffer.find(b'\xff\xd8')  # JPEG start
        end   = buffer.find(b'\xff\xd9')  # JPEG end
        if start != -1 and end != -1:
            jpg = buffer[start:end+2]
            buffer = buffer[end+2:]
            frame = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
            if frame is not None:
                yield frame

def main():
    print("Loading YOLOv8 nano model...")
    model = YOLO("yolov8n.pt")
    print("Model loaded!")

    print(f"Connecting to serial port {SERIAL_PORT}...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)
        print("Serial connected!")
    except Exception as e:
        print(f"Serial error: {e}")
        print("Check your USB connection and port name.")
        return

    last_code = ""
    last_sent_time = 0
    frame_count = 0

    while True:
        try:
            print(f"Connecting to ESP32-CAM stream: {ESP32_STREAM_URL}")
            for frame in get_frames(ESP32_STREAM_URL):
                frame_count += 1

                # Skip frames for performance
                if frame_count % FRAME_SKIP != 0:
                    cv2.imshow("YOLO Detection", frame)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        print("Quitting...")
                        cv2.destroyAllWindows()
                        ser.close()
                        return
                    continue

                # Resize to smaller resolution for faster YOLO inference
                small = cv2.resize(frame, (320, 240))

                # Run YOLO detection
                results = model(small, conf=0.5, verbose=False, imgsz=320)
                code = get_detection_code(results)

                # Annotate frame
                annotated = results[0].plot()
                annotated = cv2.resize(annotated, (frame.shape[1], frame.shape[0]))

                label_text = {
                    "P": "PERSON DETECTED",
                    "M": "MULTIPLE PEOPLE",
                    "C": "CLEAR",
                    "U": "UNKNOWN OBJECT"
                }.get(code, "")

                cv2.putText(annotated, f"[{code}] {label_text}",
                            (10, 30), cv2.FONT_HERSHEY_SIMPLEX,
                            0.8, (0, 255, 0), 2)
                cv2.imshow("YOLO Detection", annotated)

                # Send to Arduino only if changed or interval passed
                now = time.time()
                if code != last_code or (now - last_sent_time) > SEND_INTERVAL:
                    ser.write((code + "\n").encode())
                    print(f"Sent to Arduino: [{code}] {label_text}")
                    last_code = code
                    last_sent_time = now

                if cv2.waitKey(1) & 0xFF == ord('q'):
                    print("Quitting...")
                    cv2.destroyAllWindows()
                    ser.close()
                    return

        except Exception as e:
            print(f"Stream error: {e}")
            print("Retrying in 3 seconds...")
            time.sleep(3)

if __name__ == "__main__":
    main()
