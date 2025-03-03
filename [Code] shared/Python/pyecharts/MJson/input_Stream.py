import json
import re

def fileJsonOpen(URL,Way='r',code='utf-8'):
    with open(URL, Way, encoding=code) as file:
        str = file.readlines()[0]
        str = re.sub(r'^[^(]+\((.*)\);?$', r'\1', str)

        # print("JSON text:", str)
        try:
            return json.loads(str)
        except json.decoder.JSONDecodeError as e:
            print(f"JSON 解码错误: {e}")
            return None