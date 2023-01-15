import socket
import csv
import sys
import threading
from msilib import add_data
from time import sleep, time
from tkinter import *

import dearpygui.dearpygui as dpg

address_ip = 'localhost'
port = 1234

message_to_send = ""
message_to_get = ""
list_of_topics = ''
topics_list = [None]

def refresh_list_callback(topics_list_1 = [None]):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
    server_address = (address_ip, port)  # SERWER_PORT 1234
    server_socket.connect(server_address)
    data = [None]
    server_socket.sendall(b'1')  # Send data - case 1 -  to the server to get topic from
    chunk_size = (sys.getsizeof(server_socket.recv))
    if chunk_size > 0 or chunk_size < 0:
        sleep(1)
        data = server_socket.recv(chunk_size)
        list_of_topics = data.decode()  # Receive data from the server
        topics_list = list_of_topics.split(';')
        dpg.configure_item('topics', items=topics_list)
        #print(topics_list)
    else:
        print("Lista topiców jest pusta")

    server_socket.close()
    print("Topic list refreshed")


def send_message_callback():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
    server_address = (address_ip, port)
    server_socket.connect(server_address)
    message = dpg.get_value('message_to_send')
    selectedTopic = dpg.get_value('topics')
    server_socket.sendall(b'3'+b';'+ bytes((selectedTopic),'utf-8') + b';'+ bytes((message),'utf-8'))
    print("Message sent")

    #TODO : póki co max ilość znaków w wysyłanej wiadomości: 100 - zmienić


def subscribe_topic():
    # if selectedTopic == None :
        selectedTopic = dpg.get_value('topics')
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
        server_address = (address_ip, port)
        server_socket.connect(server_address)
        server_socket.sendall(b'2' + b';' + bytes((selectedTopic),'utf-8'))

        while True:
            chunk_size = (sys.getsizeof(server_socket.recv))
            data = server_socket.recv(chunk_size)
            message = data.decode()  # Receive data from the server
            print(message)
            # dpg.configure_item('messages_from_subscribed_topics', items=topics_list)
            dpg.set_value('message', dpg.get_value('message') + '\n' + message)
    # else:
    #     print('jest aktywna subskrybcja:'+ dpg.get_value('topics'))

    #TODO : działa tylko pierwsza subskrypcja
    # jak wejde do tej subskrybcji to nie mogę robić nic innego, trzeba z niej wyjść,
    # albo zostawić w niej jeden wątek.



def unsubscribe_topic():


    #TODO zakończyć pętlę while w funkcji def subscribe
    print("xxx")


def add_topic_callback(sender):
    added_topic_name = dpg.get_value('new_topic_input')
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
    server_address = (address_ip, port)
    server_socket.connect(server_address)
    server_socket.sendall(b'4' + b';' + bytes((added_topic_name), 'utf-8'))
    server_socket.close()
    dpg.set_value('new_topic_input','')
    print("Topic "+added_topic_name+" added")

def delete_topic_callback(sender):
    selectedTopic_to_delete = dpg.get_value('topics')
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
    server_address = (address_ip, port)
    server_socket.connect(server_address)
    server_socket.sendall(b'5' + b';' + bytes((selectedTopic_to_delete), 'utf-8'))
    server_socket.close()
    print("Topic "+selectedTopic_to_delete+" removed")


def close_client():
    print("Client closed")
    dpg.destroy_context()


dpg.create_context()
dpg.create_viewport(title='Client publish-subscribe app')
dpg.setup_dearpygui()

with dpg.window(tag="Primary Window",label="Client publish-subscribe app",autosize=True):
    dpg.set_primary_window("Primary Window", True)
    dpg.set_viewport_height(700)
    dpg.set_viewport_width(700)
    dpg.add_text("Hello, User!")
    dpg.add_text("Here You can read reviews on the topic you are interested in:\n\n")
    dpg.add_text("Topics:\n")
    dpg.add_listbox(label='', tag='topics', items=topics_list, num_items=5)
    dpg.add_button(label="Refresh list", callback=refresh_list_callback)
    dpg.add_button(label="Subscribe topic", callback=subscribe_topic)
    dpg.add_button(label="Unsubscribe topic", callback=unsubscribe_topic)
    dpg.add_button(label="Delete topic", callback=delete_topic_callback)
    dpg.add_text("Input new topic name:")
    dpg.add_input_text(label="", tag='new_topic_input')
    dpg.add_button(label="Add topic", callback=add_topic_callback)
    dpg.add_text("Messages from topics:")
    dpg.add_text(message_to_get,color='green',tag='message')
    dpg.add_text("Add message to topic:")
    dpg.add_input_text(label="", tag='message_to_send')
    dpg.add_button(label="Send message", callback=send_message_callback)
    dpg.add_button(label="Close app", callback=close_client)


dpg.show_viewport()
dpg.start_dearpygui()
dpg.destroy_context()
