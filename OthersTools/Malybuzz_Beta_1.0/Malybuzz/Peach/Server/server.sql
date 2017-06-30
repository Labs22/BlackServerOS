
-- 
-- Copyright (c) 2007 Michael Eddington
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a copy 
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights 
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
-- copies of the Software, and to permit persons to whom the Software is 
-- furnished to do so, subject to the following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in	
-- all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.
-- 

-- Authors:
--   Michael Eddington (mike@phed.org)

-- $Id: server.sql 279 2007-05-08 01:21:58Z meddingt $

use peach;

-- GRANT ALL PRIVILEGES ON peach.* TO 'peach'@'localhost' IDENTIFIED BY 'peach';

drop table if exists testrun;
create table testrun (

	testrunid	int primary key auto_increment,
	
	runid		varchar(36),
	client		varchar(255),
	
	finished	tinyint default 0,

	starttime	timestamp,
	finishtime	timestamp,
	
	index(runid),
	index(starttime),
	index(finishtime)
	
	);

drop table if exists test;
create table test (
	
	testid		int primary key auto_increment,
	
	runid		varchar(36),
	
	testnum		int,
	testname 	varchar(255),
	
	testdata	longblob,
	testresult	longblob,
	
	starttime	timestamp,
	finishtime	timestamp,
	
	index(runid),
	index(starttime),
	index(finishtime)
	
	);

-- end
