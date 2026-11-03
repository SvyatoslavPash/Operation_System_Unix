import pika
import redis
import json
import hashlib
import time
import os

def get_connection():
	whule True:
		try:
			return pika.BlockingConnection(pika.ConnectionParameters(host='rabbitmq'))
		except pika.exceptions.AMQPConnectionError:
			print("Worker ожидает RabbitMQ...")
			time.sleep(2)

connection = get_connection()
channel = connection.channel()
channel.queue_declare(queue='hash_queue')
redis_client = redis.Redis(host='redis', port=6379, db=0)

def callback(ch, method, properties, body):
	task = json.loads(body)
	task_id = task['task_id']
	text = task['text']

	print(f" [x] Worker {os.uname()[1]} get task {task_id}")

	time.sleep(2)
	hash_val = hashlib.sha256(text.encode('utf-8')).hexdigest()

	redis_client.set(task_id, hash_val)
	print(f" [x] Task {task_id} complete.")

channel.basic_consume(queue='hash_queue', on_message_callback=callback, auto_ack=True)

print(f' [*] Worker {os.uname()[1]} launched. Waiting for tasks...')
channel.start_consuming()
