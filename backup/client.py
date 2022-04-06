import requests

url = "http://127.0.0.1:9997/"
params = {"search": "西红柿炒鸡蛋"}
params2 = {'fun': 'switch_left'}
params3 = {'video': 'ON'}
res = requests.get(url=url, params=params2)
print(res.text)
