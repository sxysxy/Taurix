#!/usr/bin/python3

try:
    from PyQt5.QtCore import *
    from PyQt5.QtGui import *
    from PyQt5.QtWidgets import *
except ImportError:
    print("PyQt5 not installed, please run pip3 install pyqt5 first")

import sys
import os
import copy
import json

class ConfigTreeItem(QTreeWidgetItem):
    def __init__(self, json_bind):
        super(QTreeWidgetItem, self).__init__()
        self.json_bind = json_bind

class ConfigListItem(QListWidgetItem):
    def __init__(self, description):
        super(QListWidgetItem, self).__init__()
        self.description = description


class ConfigWindow(QMainWindow):
    def __init__(self, parent=None):
        super(QMainWindow, self).__init__(parent)
        self.resize(self.default_width(), self.default_height())
        #move to center
        self.move((QDesktopWidget().size().width() - self.width()) / 2, 
                    (QDesktopWidget().size().height() - self.height()) / 2)
        self.setWindowTitle("Configure")

        self.layout_main = QVBoxLayout(self)
        self.layout_params_setting = QHBoxLayout(self)
        self.layout_buttons = QHBoxLayout(self)
        self.layout_main.addLayout(self.layout_params_setting, 10)
        self.layout_main.addLayout(self.layout_buttons, 1)

        self.class_view = QTreeWidget(self)
        self.class_view.setHeaderHidden(True)
        self.class_view.setAutoScroll(True)
        self.layout_params_setting.addWidget(self.class_view, 1)

        self.layout_param_view = QVBoxLayout(self)
        self.layout_params_setting.addLayout(self.layout_param_view, 1)
        
        self.param_view_param = QListWidget(self)
        self.param_view_param.setAutoScroll(True)
        self.param_view_introduce = QTextEdit(self)
        self.layout_param_view.addWidget(self.param_view_param, 5)
        self.layout_param_view.addWidget(self.param_view_introduce, 3)
        self.param_view_introduce.setReadOnly(True)

        #buttons
        self.button_save_exit = QPushButton(self)
        self.button_save_exit.setText("Save&Exit")
        self.button_discard_exit = QPushButton(self)
        self.button_discard_exit.setText("Discard&Exit")
        self.button_help = QPushButton(self)
        self.button_help.setText("Help")

        self.layout_buttons.addWidget(self.button_save_exit, 1)
        self.layout_buttons.addWidget(self.button_discard_exit, 1)
        self.layout_buttons.addWidget(self.button_help, 1)

        self.layout_main.setGeometry(QRect(0, 0, self.width(), self.height()))
        self.setLayout(self.layout_main)

        self.json_data = None
        self.init_params()

        self.button_save_exit.clicked.connect(self.save_exit)
        self.button_discard_exit.clicked.connect(self.discard_exit)
        self.button_help.clicked.connect(self.help)

    def default_width(_):
        return 800

    def default_height(_):
        return 600

    def resizeEvent(self, event):
        self.layout_main.setGeometry(QRect(0, 0, event.size().width(), event.size().height()))
        event.accept()
        
    def closeEvent(self, event):
        event.accept()

    def init_params(self):
        with open("taurix-settings.json", "r") as f:
            self.json_data = json.load(f)

        def add_item(node, text, json_bind=None):
            item = ConfigTreeItem(json_bind)
            item.setText(0, text)
            if node != None:
                node.addChild(item)
            else:
                self.class_view.addTopLevelItem(item)
            return item

        def dfs_keys(node, data):
            if not "__final__" in data:
                for key in data.keys():
                    item = add_item(node, key, data[key])    
                    dfs_keys(item, data[key])

        dfs_keys(None, self.json_data)
        self.class_view.itemClicked.connect(self.switch_params)
        self.param_view_param.itemClicked.connect(self.switch_introduce)

    def save_exit(self):
        with open("taurix-settings-final.json", "w") as f:
            json.dump(self.json_data, f, indent=4)
        sys.exit(0)

    def discard_exit(self):
        sys.exit(0)

    def help(self):
        pass

    #切换树形视图中的项目时调用的函数
    def switch_params(self, item):
        if "__final__" in item.json_bind:
            self.param_view_param.clear()

            desc = dict()
            if "__description__" in item.json_bind:
                desc = item.json_bind["__description__"]  
            for key in item.json_bind.keys():
                if key != "__description__" and key != "__final__":
                    option = ConfigListItem(desc[key] if key in desc else "")
                    self.param_view_param.addItem(option)
                    if isinstance(item.json_bind[key], bool):
                        w = QWidget()
                        ly = QHBoxLayout()
                        l = QLabel()
                        l.setText(key)
                        l.setFixedHeight(24)
                        cb = QCheckBox()
                        cb.setCheckState(item.json_bind[key] > 0)
                        def __change_state(s):
                            #print(l.text())
                            item.json_bind[l.text()] = s > 0
                        cb.stateChanged.connect(__change_state)
                        ly.addWidget(l,100)
                        ly.addWidget(cb, 1)
                        w.setLayout(ly)
                        self.param_view_param.setItemWidget(option, w) 
                    else:
                        w = QWidget()
                        ly = QHBoxLayout()
                        l2 = QLabel()
                        l2.setText(key)
                        l2.setFixedHeight(24)
                        le = QLineEdit()
                        ly.addWidget(l2, 1)
                        ly.addWidget(le, 3)
                        le.setText(item.json_bind[key])
                        le.setAlignment(Qt.AlignRight)
                        def __change_text(s):
                            #print(l2.text())
                            item.json_bind[l2.text()] = s
                        le.textChanged.connect(__change_text)
                        w.setLayout(ly)
                        self.param_view_param.setItemWidget(option, w)
    
    def switch_introduce(self, item):
        self.param_view_introduce.setText(item.description)

if __name__ == "__main__":
    try:
        QtCore.QCoreApplication.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling) #设置高dpi下自适应调整
    except:
        pass
    app = QApplication(sys.argv)
    w = ConfigWindow()
    w.show()
    sys.exit(app.exec())
