from ultralytics import YOLO
import cv2
import pika
import os
import requests

RABBITMQHOST = os.environ["RABBITMQDOMAIN"]
RABBITMQUSER = os.environ["RABBITMQUSER"]
RABBITMQPASS = os.environ["RABBITMQPASS"]
CAMERAURL = os.environ["CAMERAURL"]
CAMERATOKEN = os.environ["CAMERATOKEN"]

# Load YOLOv8 model
model = YOLO("yolov8m_bin_trained.pt")

# Get image from camera
headers = {"Authorization": "Bearer " + CAMERATOKEN}
response = requests.get(CAMERAURL, headers=headers)

print(response.status_code)

if response.status_code == 200:
    with open("pictures/captures/captured_image.jpg", "wb") as file:
        file.write(response.content)
        print("Image successfully saved as 'captured_image.jpg'")

image_path = "pictures/captures/captured_image.jpg"
image = cv2.imread(image_path)

# Run inference
results = model(image)
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
