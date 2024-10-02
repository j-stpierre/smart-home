from ultralytics import YOLO
import cv2

# Load YOLOv8 model
model = YOLO('yolov8m_bin_trained.pt')  # Use 'yolov8n.pt' for nano model

# Load an image
image_path = 'pictures/captures/garbage-day.png'
image = cv2.imread(image_path)

# Run inference
results = model(image)
result = results[0]

bin_found = False

for box in result.boxes:
    if box.cls == 0:
        bin_found = True

print(bin_found)


# K8s job time frame for when to pull camera image
# Pull camera image
# Call inference function
# If no bin detected produce event
