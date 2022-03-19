import requests

url = "http://127.0.0.1:10001/"
params = {"fun": "search", 'recipe': 'test'.encode('utf-8')}
params2 = {'fun': 'switch_left'}
params3 = {'video': 'ON'}
res = requests.get(url=url, params=params3)
print(res.text)
