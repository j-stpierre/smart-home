package slack

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"notifier/config"
)

func SendSlackMessage(cfg *config.Config, message string) (err error) {

	jsonBody := []byte(fmt.Sprintf("{\"text\": \"%s\"}", message))
	bodyReader := bytes.NewReader(jsonBody)

	req, err := http.NewRequest(http.MethodPost, cfg.SLACKWEBHOOK, bodyReader)
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
