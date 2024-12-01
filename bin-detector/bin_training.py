from ultralytics import YOLO

# Step 1: Load YOLOv8 model (pretrained)
model = YOLO("yolo11m.pt")  # 'n' for nano (small and fast)

# Step 2: Train with custom dataset
model.train(
    data="data.yaml",  # Path to the YAML dataset config
    epochs=100,  # Number of epochs to train
    imgsz=640,  # Image size to resize images to
    device="cpu",  # Use GPU (device=0) or CPU (device='cpu')
)

# Step 3: Evaluate performance on the validation set
model.val()

# Perform object detection on an image
results = model("pictures/captures/snapshot.jpg")
results[0].show()

# Step 4: Save the model (optional, after training)
model.save("yolo11m_bin_trained.pt")
