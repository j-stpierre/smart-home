package main

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"time"

	amqp "github.com/rabbitmq/amqp091-go"
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
			triage(d)
		}
	}()

	log.Printf(" [*] Waiting for events. To exit press CTRL+C")
	<-forever
}

func triage(message amqp.Delivery) {
	if message.Headers["source"] == "bindetector" {
		err := sendSlackMessage(string(message.Body))
		if err != nil {
			log.Printf("Message %s, could not be sent to slack: %s", message.Body, err)
		} else {
			log.Printf("Message %s, was sent to slack", message.Body)
		}
	} else {
		log.Printf("Message %s, was ignored", message.Body)
	}
}

func sendSlackMessage(message string) (err error) {

	jsonBody := []byte(fmt.Sprintf("{\"text\": \"%s\"}", message))
	bodyReader := bytes.NewReader(jsonBody)

	req, err := http.NewRequest(http.MethodPost, slackwebhook, bodyReader)
	req.Header.Set("Content-Type", "application/json")

	client := http.Client{}

	res, err := client.Do(req)
	if err != nil {
		fmt.Printf("client: error making http request: %s\n", err)
		return errors.New(err.Error())
	}

	log.Printf("client: status code: %d\n", res.StatusCode)
	resBody, err := io.ReadAll(res.Body)
	if err != nil {
		log.Printf("client: could not read response body: %s\n", err)
	}
	log.Printf("client: response body: %s\n", resBody)
	return nil
}
