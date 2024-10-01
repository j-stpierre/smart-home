package rabbitmq

import (
	"errors"
	"log"
	"notifier/config"

	amqp "github.com/rabbitmq/amqp091-go"
)

func failOnError(err error, msg string) {
	if err != nil {
		log.Panicf("%s: %s", msg, err)
	}
}

type RabbitMQ struct {
	Conn    *amqp.Connection
	Channel *amqp.Channel
	Queue   string
}

func NewConsumer(cfg *config.Config) (*RabbitMQ, error) {

	conn, err := amqp.Dial("amqp://" + cfg.RABBITMQUSER + ":" + cfg.RABBITMQPASS + "@" + cfg.RABBITMQDOMAIN + ":" + cfg.RABBITMQPORT + "/")
	failOnError(err, "Failed to connect to RabbitMQ")

	ch, err := conn.Channel()
	failOnError(err, "Failed to open a channel")

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

	return &RabbitMQ{conn, ch, q.Name}, nil
}

func (r *RabbitMQ) Close() {
	r.Conn.Close()
	r.Channel.Close()
}

func (r *RabbitMQ) Consume() (<-chan amqp.Delivery, error) {
	msgs, err := r.Channel.Consume(
		r.Queue, // queue
		"",      // consumer
		true,    // auto-ack
		false,   // exclusive
		false,   // no-local
		false,   // no-wait
		nil,     // args
	)

	if err != nil {
		log.Printf("Could not start consuming messages")
		return nil, errors.New(err.Error())
	}
	return msgs, nil
}
