# smart-home

## Hub (Raspberry Pi)
### K3s
Follow steps at https://docs.k3s.io/quick-start \
Be sure to add `cgroup_memory=1 cgroup_enable=memory` to the end of `/boot/firmware/cmdline.txt` on the raspberry pi

Add secret for private docker repo with the following command \
`Set the env variables locally to substitute before running`
```
kubectl create secret docker-registry my-registry-secret \
  --docker-server=docker.io/jstpierre24/smart-home \
  --docker-username=$DOCKERUSER \
  --docker-password=$DOCKERPASS \
  --docker-email=$DOCKEREMAIL
  ```
Add the following to each container config in the required kubernetes manifests
```
imagePullSecrets:
  - name: my-registry-secret
```

### RabbitMQ
Install RabbitMQ in a new namespace
`kubectl create namespace rabbitmq`

Install RabbitMQ with the RabbitMQ Operator as seen here: https://www.rabbitmq.com/kubernetes/operator/quickstart-operator

`kubectl apply -f manifests/rabbitmq/rabbitmq.yaml`
```
apiVersion: rabbitmq.com/v1beta1
kind: RabbitmqCluster
metadata:
  name: rabbitmq
```


### Notifier Service
Ensure the following environment variables are set:
- SLACKWEBHOOK
- RABBITMQPORT
- RABBITMQPASS
- RABBITMQUSER

To build a service that will run on raspberry pi ARM64 you need to build it similar to the follow:
```
docker buildx build --platform linux/arm64 -t jstpierre24/smart-home:notifier-v1.0.0 .
```

### Bin Detector Service
Ensure the following enviornment variables are set:
- RABBITMQPORT
- RABBITMQPASS
- RABBITMQUSER
- CAMERAURL
- CAMERATOKEN

To train, use roboflow to annotate the images and export the version as yolov8 which will give you a txt file per image

### ESP32-CAM
Upload code from camera foler to esp32.
Requires a sercrets.h file which includes the following variables
```
#define WIFI_SSID
#define WIFI_PASSWORD
#define BEARER_TOKEN
```
