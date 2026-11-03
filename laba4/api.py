from flask import Flask, request, jsonify
import pika
import redis
import uuid
import json
import time
import os

app = Flask(__name__)

def get_rabbitmq_channel():
	while True:
		try:
			connection = pika.BlockingConnection(pika.ConnectionParameters(host='rabbitmq'))
			return connection, connection.channel()
		except pika.exceptions.AMQPConnectionError:
			print("Ожидание RabbitMQ...")
			time.sleep(2)

connection, channel = get_rabbitmq_channel()
channel.queue_declare(queue='hash_queue')
redis_client = redis.Redis(host='redis', port=6379, db=0)

@app.route('/submit', methods=['POST'])
def submit():
	data = request.get_json()
	text = data.get('text', '')

	task_id = str(uuid.uuid4())
	task = {'task_id': task_id, 'text': text}

	channel.basic_publish(
		exchange='',
		routing_key='hash_queue',
		body=json.dumps(task)
	)

	return jsonify({'task_id': task_id, 'status': 'queued', 'handled_by': os.uname()[1]}), 202

@app.route('/result/<task_id>', methods=['GET'])
def get_result(task_id):
	result = redis_client.get(task_id)
	if result:
		return jsonify({'task_id': task_id, 'hash': result.decode('utf-8')}), 200
	else:
		return jsonify({'task_id': task_id, 'status': 'processing', 'handled_by': os.uname()[1]}), 404

if __name__ == '__main__':
	app.run(host='0.0.0.0', prot=5000)
