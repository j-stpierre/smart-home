from ultralytics import YOLO
import cv2
import pika
import os

RABBITMQHOST = os.environ['RABBITMQDOMAIN']
RABBITMQUSER = os.environ['RABBITMQUSER']
RABBITMQPASS = os.environ['RABBITMQPASS']

# Load YOLOv8 model
model = YOLO('yolov8m_bin_trained.pt')

# Load an image
image_path = 'pictures/captures/no-bin-day.png'
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
    connection = pika.BlockingConnection(pika.ConnectionParameters(host=RABBITMQHOST,credentials=credentials))
    channel = connection.channel()

    headers = {
    'source': 'bindetector',
    }

    properties = pika.BasicProperties(
        headers=headers
    )

    channel.basic_publish(exchange='events', routing_key='', body='No bin detected', properties=properties)
    print(" [x] Sent 'No bin detected'")
    connection.close()
