import os
import re

class MenuLoader:
    def __init__(self):
        self.menu_content = []
        self.load_menu()

    # 读一个文件 返回一个包含文件句子的列表
    def read_file_lines(self, goal):
        pwd = os.getcwd()
        cur_path = os.path.join(pwd, '../menu', goal)
        res = []
        with open(cur_path, 'r', encoding='UTF-8') as f:
            # while True:
            #     tem = f.readline()
            #     if tem == '':
            #         break
            #     else:
            #         tem = tem.replace('\n', '')
            #         res.append(tem)
            while True:
                tmp = f.readline()
                if tmp == '## 操作\n':
                    break
                elif tmp == '':
                    break
            
            while True:
                tmp = f.readline()
                if tmp == '' or tmp == '## 附加内容\n':
                    break
                elif tmp == '\n':
                    continue
                else:
                    tmp = tmp.replace('\n', '')
                    tmp = re.sub('(\d+. )|(- )', '', tmp)
                    res.append(tmp)
        return res

    # 找一个食谱，找到返回内容列表，否则返回None
    def search_recipe(self, goal):
        for item in self.menu_content:
            if goal in item:
                reader = self.read_file_lines(item)
                return reader
        return None

    # 测试接口
    def load_recipe(self, recipe_goal):
        recipe = self.search_recipe(recipe_goal)
        print(recipe)

    def load_menu(self):
        pwd = os.getcwd()
        cur_path = os.path.join(pwd, '../menu')
        content = os.listdir(cur_path)
        for item in content:
            self.menu_content.append(item)


if __name__ == '__main__':
    loader = MenuLoader()
    print(loader.read_file_lines('西红柿炒鸡蛋.md'))
