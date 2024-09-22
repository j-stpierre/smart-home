package slack_integration_test

import (
	"encoding/json"
	"io"
	"net/http"
	"notifier/config"
	"notifier/internal/slack"
	"os"
	"testing"

	"github.com/google/uuid"
)

type Messages struct {
	Text string `json:"text"`
}

type Response struct {
	Messages []Messages `json:"messages"`
}

func TestSendToSlack(t *testing.T) {
	testConfig := &config.Config{
		SLACKWEBHOOK: os.Getenv("TESTSLACKWEBHOOK"),
	}

	TOKEN := os.Getenv("SLACKTOKEN")
	TESTCHANNELID := os.Getenv("TESTCHANNELID")

	expect := "Test message " + uuid.New().String()
	slack.SendSlackMessage(testConfig, expect)

	req, err := http.NewRequest(http.MethodGet, "https://slack.com/api/conversations.history?channel="+TESTCHANNELID+"&limit=1", nil)
	req.Header.Set("Authorization", "Bearer "+TOKEN)

	client := http.Client{}
	res, err := client.Do(req)
	if err != nil {
		t.Errorf("client: error making http test request: %s\n", err)
	}

	got, err := io.ReadAll(res.Body)
	if err != nil {
		t.Errorf("Could not read response body: %s\n", err)
	}

	var result Response
	err = json.Unmarshal(got, &result)
	if err != nil {
		t.Errorf("Could not parse response body of test check: %s\n", err)
	}

	lastMessage := result.Messages[0].Text
	if string(lastMessage) != expect {
		t.Errorf("Got slack message: %s, does not equal: %s", got, expect)
	}
}
