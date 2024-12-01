from ultralytics import YOLO
import cv2
import pika
import os

RABBITMQHOST = os.environ["RABBITMQDOMAIN"]
RABBITMQUSER = os.environ["RABBITMQUSER"]
RABBITMQPASS = os.environ["RABBITMQPASS"]
CAMERAURL = os.environ["CAMERAURL"]
CAMERAAUTH = os.environ["CAMERAAUTH"]

# Load YOLOv8 model
model = YOLO("yolo11m_bin_trained.pt")

# Get image from camera
rtsp_url = "rtsp://" + CAMERAAUTH + "@" + CAMERAURL

# Create a VideoCapture object
cap = cv2.VideoCapture(rtsp_url)

# Check if the stream is opened successfully
if not cap.isOpened():
    print("Error: Could not open RTSP stream")
else:
    # Read a frame (snapshot)
    ret, frame = cap.read()

    if ret:
        cv2.imwrite("pictures/captures/snapshot.jpg", frame)
    else:
        print("Error: Could not retrieve frame")

# Release the VideoCapture object
cap.release()

image_path = "pictures/captures/snapshot.jpg"
image = cv2.imread(image_path)

down_points = (640, 640)
resized_down = cv2.resize(image, down_points, interpolation=cv2.INTER_LINEAR)

# Run inference
results = model(resized_down)
result = results[0]

bin_found = False

for box in result.boxes:
    if box.cls == 0:
        bin_found = True

# If False send message to rabbitmq
if not bin_found:

    credentials = pika.PlainCredentials(RABBITMQUSER, RABBITMQPASS)
    connection = pika.BlockingConnection(
        pika.ConnectionParameters(host=RABBITMQHOST, credentials=credentials)
    )
    channel = connection.channel()

    headers = {
        "source": "bindetector",
    }

    properties = pika.BasicProperties(headers=headers)

    channel.basic_publish(
        exchange="events", routing_key="", body="No bin detected", properties=properties
    )
    print(" [x] Sent 'No bin detected'")
    connection.close()
