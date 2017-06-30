/* Crypton Server, Copyright 2013 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
*/

'use strict';

var https = require('https');
var express = require('express');
var path = require('path');
var bodyParser = require('body-parser');
var errorhandler = require('errorhandler');
var appsec = require('lusca');
var fs = require('fs');
var cors = require('cors');
var util = require('./lib/util');
var version = require('./package.json').version;
var redis = require('redis');

// logging
var colors = require('colors');
var expressWinston = require('express-winston');
var winston = require('winston'); // for transports.Console
winston.emitErrs = true;

var app = process.app = module.exports = express();

// Don't expose 'X-Powered-By: Express' response header.
app.disable('x-powered-by');

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

var myLogTransports = [];

if (process.env.NODE_ENV === 'production') {
  myLogTransports.push(new winston.transports.File(
    {
      level: 'warn',
      filename: './logs/production.log',
      handleExceptions: true,
      json: true,
      maxsize: 104857600, // 100MB
      maxFiles: 5,
      colorize: false,
      tailable: true,
    }
  ));
} else if (process.env.NODE_ENV === 'development') {
  myLogTransports.push(new winston.transports.Console(
    {
      timestamp: true,
      level: 'info',
      handleExceptions: true,
      json: false,
      colorize: true,
      prettyPrint: true,
    }
  ));
  myLogTransports.push(new winston.transports.File(
    {
      level: 'info',
      filename: './logs/development.log',
      handleExceptions: true,
      json: true,
      maxsize: 5242880, // 5MB
      maxFiles: 5,
      colorize: false,
      tailable: true,
    }
  ));
} else if (process.env.NODE_ENV === 'test') {
  myLogTransports.push(new winston.transports.Console(
    {
      silent: true,
    }
  ));
  myLogTransports.push(new winston.transports.File(
    {
      level: 'info',
      filename: './logs/test.log',
      handleExceptions: true,
      json: true,
      maxsize: 5242880, // 5MB
      maxFiles: 5,
      colorize: false,
      tailable: true,
    }
  ));
} else {
  console.error('ERROR : NODE_ENV must be set to one of: development | test | production');
  process.exit(1);
}

var logger = new winston.Logger({
  transports: myLogTransports,
  exitOnError: false,
});

global.logger = logger;

// express-winston logger makes sense BEFORE the router.
app.use(expressWinston.logger({
  transports: myLogTransports,
  meta: true, // optional: control whether you want to log the meta data about the request (default to true)
  msg: 'HTTP {{req.method}} {{req.url}}', // optional: customize the default logging message. E.g. "{{res.statusCode}} {{req.method}} {{res.responseTime}}ms {{req.url}}"
  expressFormat: true, // Use the default Express/morgan request formatting, with the same colors. Enabling this will override any msg and colorStatus if true. Will only output colors on transports with colorize set to true
  colorStatus: true, // Color the status code, using the Express/morgan color palette (default green, 3XX cyan, 4XX yellow, 5XX red). Will not be recognized if expressFormat is true
}));

app.config = require('./lib/config');

// logger.info('app.config', app.config);

app.datastore = require('./lib/storage');

var redisSession = require('./redis-session')({
  debug: true,
  connection: {
    port: app.config.redis.port,
    host: app.config.redis.host,
    db: app.config.redis.db,
  },
});

app.redisSession = redisSession;

app.SERVER_VERSION = version;

// parse application/x-www-form-urlencoded
app.use(bodyParser.urlencoded({ extended: false }));

// parse application/json
app.use(bodyParser.json());

app.use(cors({
  credentials: true,
  origin: function(origin, callback) {
    callback(null, true);
  },
}));

if (app.config.securityHeaders) {
  try {
    var luscaObj = app.config.securityHeaders;

    if (typeof luscaObj === 'object') {
      app.use(appsec(luscaObj));
    } else {
      throw new Error('securityHeaders must be an Object conforming to the Lusca security config.  See : https://github.com/krakenjs/lusca');
    }
  } catch (ex) {
    logger.error(ex);

    // Exit in an orderly fashion
    process.exit(1);
  }
} else {
  logger.warn('No securityHeaders set in app.config! Security Headers are required to run this application in production. CSP connect-src is limited to localhost');

  // A very strict CSP, CSRF enabled and xframe options as sameorigin.
  app.use(appsec({
    csrf: false,
    csp: {
      policy:{
        'default-src': "'self' blob:",
        'connect-src': "wss://localhost localhost",
        'script-src': "'self'",
        'img-src': "'self'",
        'style-src': "'self'",
        'font-src': "'self'",
        'object-src': "'self'",
      },
    },
    xframe: 'SAMEORIGIN',
    hsts: {
      maxAge: 31536000,
    },
  }));
}

// Do not cache responses.
// XXXddahl: We are using a sledge hammer here, perhaps we can refine this at some point?
app.use(function(req, res, next) {
  res.set({
    'Cache-Control': 'no-cache',
  });
  next();
});

if (app.config.appPath) {
  var appPath = path.resolve(process.cwd(), app.config.appPath);
  app.use(express.static(appPath));
  app.use(express.static(__dirname + '/../client/dist'));
}

app.options('/*', function(req, res) {
  res.send('');
});

app.start = function start() {
  logger.info('starting HTTPS server');

  var privateKeyPath = path.resolve(process.cwd(), app.config.privateKey);
  var certificatePath = path.resolve(process.cwd(), app.config.certificate);
  var privateKeyExists = fs.existsSync(privateKeyPath);
  var certificateExists = fs.existsSync(certificatePath);
  var privateKeyRealPath = privateKeyExists ? privateKeyPath : __dirname + '/config/privateKeyExample.pem';
  var certificateRealPath = certificateExists ? certificatePath : __dirname + '/config/certificateExample.pem';

  var options = {
    key: fs.readFileSync(privateKeyRealPath).toString(),
    cert: fs.readFileSync(certificateRealPath).toString(),
  };

  app.port = app.config.port || 443;
  app.server = https.createServer(options, app);
  app.server.listen(app.port, function() {
    logger.info('HTTPS server listening on port ' + app.port);
    require('./lib/sockets');
  });
};

require('./routes');

// load after the routes
if (process.env.NODE_ENV === 'development') {
  // only use in development
  app.use(errorhandler());
}

// express-winston errorLogger makes sense AFTER the router.
app.use(expressWinston.errorLogger({
  transports: myLogTransports,
}));

function handleError(err) {
  logger.error('handleError : ', err, err.stack);
  process.exit(1);
}

process.on('uncaughtException', handleError);
