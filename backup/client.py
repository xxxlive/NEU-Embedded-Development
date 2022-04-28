import requests
import pyttsx3 as tts

res = []
s = "fadsf"

engine = tts.init()
for item in engine.getProperty("voices"):
    if str(item.languages[0]).find('zh') != -1:
        print(item)
        engine.setProperty('voice', item.id)
        # engine.setProperty('gender', 'VoiceGenderMale')
        engine.say("奥迪双钻，我的伙伴")
        engine.runAndWait()
# url = "http://127.0.0.1:9997/"
# params = {"search": "西红柿炒鸡蛋"}
# params2 = {'fun': 'switch_left'}
# params3 = {'video': 'ON'}
# res = requests.get(url=url, params=params2)
# print(res.text)
