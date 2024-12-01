from ultralytics import YOLO
import cv2

# Load YOLOv8 model
model = YOLO("yolo11m_bin_trained.pt")

image_path = "pictures/captures/snapshot.jpg"
image = cv2.imread(image_path)

resized_down = cv2.resize(image, (640, 640))

# Run inference
results = model(image, conf=0.25)

for result in results:
    for box in result.boxes:
        x1, y1, x2, y2 = map(int, box.xyxy[0])  # Bounding box coordinates
        label = model.names[int(box.cls)]  # Class label
        confidence = box.conf[0].item()  # Confidence score
        # Draw the bounding box
        cv2.rectangle(image, (x1, y1), (x2, y2), (0, 255, 0), 2)
        # Display label and confidence
        label_text = f"{label} {confidence:.2f}"
        cv2.putText(
            image,
            label_text,
            (x1, y1 - 10),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.5,
            (255, 0, 0),
            2,
        )

# Show the result
cv2.imshow("YOLOv8 Detection", image)
cv2.waitKey(0)
cv2.destroyAllWindows()
