from time import sleep
import Jetson.GPIO as GPIO
import requests


def main():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(38, GPIO.IN)
    while True:
        up = GPIO.input(38)
        if up == 1:
            url2 = "http://127.0.0.1:9999/"
            params3 = {'clear': '我在'}
            url1 = "http://127.0.0.1:10001/"
            params = {"video": "ON"}
            requests.get(url=url2, params=params3)
            sleep(0.5)
            requests.get(url=url1, params=params)
            sleep(2)
        sleep(1)


if __name__ == '__main__':
    main()
