#!/usr/bin/env python

from Peach.Generators.dictionary  import *
from Peach.Server.client import *

client = PeachClient('http://127.0.0.1:8000')

client.RunStarting()

try:
	gen = List(None, ['1', '2', '3'])
	cnt = 0
	while True:
		val = gen.getValue()
		client.StartTest('List Test', cnt, val)
		print val
		client.EndTest('List Test', cnt)
		gen.next()
		cnt += 1

except generator.GeneratorCompleted:
	pass

client.RunFinished()
