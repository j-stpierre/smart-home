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

### Influx DB
Install InfluxDB by following the instructions here \
https://docs.influxdata.com/platform/install-and-deploy/deploying/kubernetes/ \
Once in influxdb UI you can use the telegraf plugin section and select rabbitMQ, where it will create a sample config that needs to be translated into the manifest file under the config section, along with the influx token that gets created

### Telegraf
Follow install instructions here \
https://github.com/influxdata/helm-charts/tree/master/charts/telegraf \
Use yaml config found in manifests/telegraf \
Create a secret called `telegrafsecrets` which contains a secret for rabbitmq password as well as influxdb token

### Grafana For Telemetry
Deploy following the instructions here \
https://grafana.com/docs/grafana/latest/setup-grafana/installation/helm/ \
Usings the manifest in the manifests/grafana section as values \
To setup influx connector use Authorization header with Token <TOKEN> as the value and database is the bucket you want to query.

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
- CAMERAURL (ip:port/streampath)
- CAMERAAUTH (username:password)

To train, use roboflow to annotate the images and export the version as yolov8 which will give you a txt file per image

## Observability

