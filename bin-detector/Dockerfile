FROM python:3.11.10-slim

# Install build dependencies
RUN apt-get update && \
    apt-get install -y build-essential qt5-qmake libqt5widgets5 libqt5gui5 libqt5core5a libgl1-mesa-dev && \
    apt-get clean

WORKDIR /usr/src/app

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

CMD [ "python", "./bindetector.py" ]
