package triage

import (
	amqp "github.com/rabbitmq/amqp091-go"
	"log"
	"notifier/config"
	"notifier/internal/slack"
)

func Triage(cfg *config.Config, message amqp.Delivery) {
	if message.Headers["source"] == "bindetector" {
		err := slack.SendSlackMessage(cfg, string(message.Body))
		if err != nil {
			log.Printf("Message %s, could not be sent to slack: %s", message.Body, err)
		} else {
			log.Printf("Message %s, was sent to slack", message.Body)
		}
	} else {
		log.Printf("Message %s, was ignored", message.Body)
	}
}
