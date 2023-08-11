import requests
with open("Temp.ofbwyx","r",encoding="UTF-8") as f:
    url=f.read()
    f.close()
head = {"User-Agent":"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36"}
r=requests.get(url=url,headers=head)
print(r)
with open("Temp.ofbwyx","w",encoding="UTF-8") as f:
    f.write(r.text)
    f.close()
