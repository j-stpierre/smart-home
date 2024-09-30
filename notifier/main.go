package main

import (
	"log"
	"notifier/config"
	"notifier/internal/rabbitmq"
	"notifier/internal/triage"
)

func main() {

	cfg, err := config.LoadConfig()
	if err != nil {
		log.Printf("Error loading config")
	}

	r, err := rabbitmq.NewConsumer(cfg)

	msgs, err := r.Consume()

	go func() {
		for d := range msgs {
			log.Printf("Message received")
			triage.Triage(cfg, d)
		}
	}()

	log.Printf(" [*] Waiting for events. To exit press CTRL+C")
	forever := make(chan bool)
	<-forever

}
