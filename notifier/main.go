package main

import (
	amqp "github.com/rabbitmq/amqp091-go"
	"log"
	"notifier/config"
	"notifier/internal/triage"
	"os"
)

func failOnError(err error, msg string) {
	if err != nil {
		log.Panicf("%s: %s", msg, err)
	}
}

var port = os.Getenv("RABBITMQPORT")
var username = os.Getenv("RABBITMQUSER")
var password = os.Getenv("RABBITMQPASS")
var slackwebhook = os.Getenv("SLACKWEBHOOK")

func main() {

	cfg, err := config.LoadConfig()

	//move to rabbit confi
	conn, err := amqp.Dial("amqp://" + username + ":" + password + "@localhost:" + port + "/")
	failOnError(err, "Failed to connect to RabbitMQ")
	defer conn.Close()

	ch, err := conn.Channel()
	failOnError(err, "Failed to open a channel")
	defer ch.Close()

	q, err := ch.QueueDeclare(
		"",    // name
		false, // durable
		false, // delete when unused
		true,  // exclusive
		false, // no-wait
		nil,   // arguments
	)
	failOnError(err, "Failed to declare a queue")

	err = ch.QueueBind(
		q.Name,   // queue name
		"",       // routing key
		"events", // exchange
		false,
		nil,
	)
	failOnError(err, "Failed to bind a queue")

	msgs, err := ch.Consume(
		q.Name, // queue
		"",     // consumer
		true,   // auto-ack
		false,  // exclusive
		false,  // no-local
		false,  // no-wait
		nil,    // args
	)
	failOnError(err, "Failed to register a consumer")

	var forever chan struct{}

	go func() {
		for d := range msgs {
			triage.Triage(cfg, d)
		}
	}()

	log.Printf(" [*] Waiting for events. To exit press CTRL+C")
	<-forever
	//end of rabbit config

}
