package config

import (
	"os"
)

type Config struct {
	RABBITMQDOMAIN string
	RABBITMQUSER   string
	RABBITMQPORT   string
	RABBITMQPASS   string
	SLACKWEBHOOK   string
}

func LoadConfig() (*Config, error) {
	config := &Config{
		RABBITMQDOMAIN: os.Getenv("RABBITMQDOMAIN"),
		RABBITMQUSER:   os.Getenv("RABBITMQUSER"),
		RABBITMQPORT:   os.Getenv("RABBITMQPORT"),
		RABBITMQPASS:   os.Getenv("RABBITMQPASS"),
		SLACKWEBHOOK:   os.Getenv("SLACKWEBHOOK"),
	}

	return config, nil
}
