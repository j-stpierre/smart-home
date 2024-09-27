from ultralytics import YOLO

# Step 1: Load YOLOv8 model (pretrained)
model = YOLO('yolov8m.pt')  # 'n' for nano (small and fast)

# Step 2: Train with custom dataset
model.train(
    data='data.yaml',  # Path to the YAML dataset config
    epochs=100,        # Number of epochs to train
    imgsz=640,         # Image size to resize images to
    batch=16,          # Batch size for training
    device='cpu'           # Use GPU (device=0) or CPU (device='cpu')
)

# Step 3: Evaluate performance on the validation set
model.val()

# Step 4: Save the model (optional, after training)
model.save('yolov8m_bin_trained.pt')
