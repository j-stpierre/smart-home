# smart-home

## Hub (Raspberry Pi)
### K3s
Follow steps at https://docs.k3s.io/quick-start \
Be sure to add `cgroup_memory=1 cgroup_enable=memory` to the end of `/boot/firmware/cmdline.txt` on the raspberry pi

### RabbitMQ
Install RabbitMQ with the RabbitMQ Operator as seen here: https://www.rabbitmq.com/kubernetes/operator/quickstart-operator

### Notifier Service
Ensure the following environment variables are set:
- SLACKWEBHOOK
- RABBITMQPORT
- RABBITMQPASS
- RABBITMQUSER

### Bin Detector Service
Ensure the following enviornment variables are set:
- RABBITMQPORT
- RABBITMQPASS
- RABBITMQUSER