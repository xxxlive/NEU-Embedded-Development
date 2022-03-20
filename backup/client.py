import requests

url = "http://127.0.0.1:9997/"
params = {"search": "fuckyou"}
params2 = {'fun': 'shut_down'}
params3 = {'clear': 'fasdasdfasd'}
res = requests.get(url=url, params=params)
print(res.text)
