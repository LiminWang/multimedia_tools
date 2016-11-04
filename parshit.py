#!/usr/bin/env python
import string, os, sys
import matplotlib.pyplot as plt
import numpy as np
from random import randrange
import threading

msg_file = "data"

#const
stream_count = 10

#variable
packet_enable = False
pkt_count = 0
vpkt_count = 0
apkt_count = 0
stream = []
last_pos = []
cur_id = 0
cur_stream = ""
color = ['#ff0000', '#00ff00', '#0000ff']
show_all = False

################################################################################################################
# print software about
################################################################################################################
def print_software_about():
	print '#################################################################'
	print '# Parse Shit 1.0'
	print '#'
	print '# If U find the software is a shit too, don\'t contact Chenggang'
	print '#################################################################'
	print ''
	print ''
	print ''

	
################################################################################################################
# init variable
################################################################################################################
def init_variable():
	global stream
	for i in range(0, stream_count):
		last_pos.append(0)
		stream.append({})
		stream[i]['codec_type'] = ''
		stream[i]['pts'] = []
		stream[i]['pts_time'] = []
		stream[i]['dts'] = []
		stream[i]['dts_time'] = []
		stream[i]['duration'] = []
		stream[i]['duration_time'] = []
		stream[i]['size'] = []
		stream[i]['pos'] = []
		stream[i]['flags'] = []
		color.append("#%06s" % "".join([hex(randrange(0, 255))[2:] for i in range(3)]))

################################################################################################################
# run ffprobe
################################################################################################################
def run_ffprobe():
	global show_all
	usage = sys.argv[0] + ' [filename]'

	if len(sys.argv) < 2:
		print 'Usage:'
		print usage
		sys.exit()
	elif not sys.argv[1]:
		print 'input filename = null'
		print usage
		sys.exit()
	elif not os.path.isfile(sys.argv[1]):
		print 'No find the file: %s' % sys.argv[1]
		print usage
		sys.exit()
	
	if len(sys.argv) > 2:
		show_all = True
	
	return_status = os.system('ffprobe -show_packets ' + sys.argv[1] + ' > ' + msg_file)

################################################################################################################
# parser file
################################################################################################################
def parser_file():
	global msg_file
	global stream_count
	global packet_enable
	global pkt_count
	global vpkt_count
	global apkt_count
	global stream
	global cur_id
	global cur_stream
	global last_pos

	fp = open(msg_file, 'r')

	for line in fp.readlines():
		line = line.strip("\n")
	
		if line == "[PACKET]":
			packet_enable = True
			pkt_count += 1
		elif line == "[/PACKET]":
			packet_enable = False
		

		if packet_enable:
			parts = line.split('=')
		
			if parts[0] == "codec_type":
				cur_stream = parts[1]
			elif parts[0] == "stream_index":
				cur_id = int(parts[1])
				if not stream[cur_id]['codec_type']:
					stream[cur_id]['codec_type'] = cur_stream
			elif parts[0] == "pts":
				if parts[1] == 'N/A':
					stream[cur_id]['pts'].append(0)
				else:
					stream[cur_id]['pts'].append(int(parts[1]))
			elif parts[0] == "pts_time":
				if parts[1] == 'N/A':
					stream[cur_id]['pts_time'].append(0.0)
				else:
					stream[cur_id]['pts_time'].append(float(parts[1]))
			elif parts[0] == "dts":
				if parts[1] == 'N/A':
					stream[cur_id]['dts'].append(0)
				else:
					stream[cur_id]['dts'].append(int(parts[1]))
			elif parts[0] == "dts_time":
				if parts[1] == 'N/A':
					stream[cur_id]['dts_time'].append(0.0)
				else:
					stream[cur_id]['dts_time'].append(float(parts[1]))
			elif parts[0] == "duration":
				if parts[1] == 'N/A':
					stream[cur_id]['duration'].append(0)
				else:
					stream[cur_id]['duration'].append(int(parts[1]))
			elif parts[0] == "duration_time":
				if parts[1] == 'N/A':
					stream[cur_id]['duration_time'].append(0.0)
				else:
					stream[cur_id]['duration_time'].append(float(parts[1]))
			elif parts[0] == "size":
				if parts[1] == 'N/A':
					stream[cur_id]['size'].append(0)
				else:
					stream[cur_id]['size'].append(int(parts[1]))
			elif parts[0] == "pos":
				if parts[1] == 'N/A':
					stream[cur_id]['pos'].append(last_pos[cur_id])
				else:
					stream[cur_id]['pos'].append(int(parts[1]))
					last_pos[cur_id] = int(parts[1])
			elif parts[0] == "flags":
				stream[cur_id]['flags'].append((parts[1]))
				
	fp.close()

#print stream

################################################################################################################
# draw chart
################################################################################################################
def show_chart(num, x, y):
	global stream
	global stream_count
	global color
	
	plt.figure(num)
	for i in range(0, stream_count):
		codec_type = stream[i]['codec_type']
		if not codec_type:
			continue
		plt.plot(stream[i][x], stream[i][y], color=color[i], marker='o', label=codec_type+str(i))
		plt.xlabel(x)
		plt.ylabel(y)
	plt.title(x + '-' + y)
	plt.legend()

################################################################################################################
# mutil thread
################################################################################################################
option = [[False, 'pos', 'pts'],				\
			[False, 'pos', 'pts_time'],		\
			[False, 'pos', 'dts'],				\
			[False, 'pos', 'dts_time'],		\
			[False, 'pos', 'duration'],		\
			[False, 'pos', 'duration_time'],	\
			[False, 'pos', 'size'],			\
			[False, 'pts', 'pos']				\
		]

class chartThread(threading.Thread):
	global show_all
	
	def __init__(self,threadname):
		threading.Thread.__init__(self,name=threadname)
	def run(self):
		if show_all:
			for n in range(0, len(option)):
				show_chart(n, option[n][1], option[n][2])
			plt.show()
		else:
			show_chart(0, option[0][1], option[0][2])
			plt.show()


################################################################################################################
# main
################################################################################################################

print_software_about()
init_variable()
run_ffprobe()
parser_file()

thread = chartThread('0')
thread.setDaemon(True)
thread.start()

while True:
	key = raw_input("Press q exit:")

	if key == 'q':
		sys.exit()


