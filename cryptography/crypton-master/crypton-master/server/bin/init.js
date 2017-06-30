var fs = require('fs');
var pg = require('pg').native;

// will equal envConfig or postgresUserConfig depending on context
var currentConfig;

var envConfig = JSON.parse(fs.readFileSync(__dirname + '/../config/config.' + process.env.NODE_ENV + '.json', 'utf8'));

// Clone the config and use the master postgres account for this server to
// create the new user and DB we need, and then revert back to
// using the env specific user and password for pg.
// These values come from 'GIT_ROOT/server/config/config.ENVIRONMENT.json'
var postgresUserConfig = JSON.parse(JSON.stringify(envConfig));
postgresUserConfig.database.database = 'postgres';
postgresUserConfig.database.user = postgresUserConfig.database.postgres_user;
postgresUserConfig.database.password = postgresUserConfig.database.postgres_user_password;

function connect(callback) {
  'use strict';

  console.log("\nUsing DB config: ", currentConfig.database);

  // Will connect with different users depending on what we are doing.
  pg.connect(currentConfig.database, function(err, client, done) {
    if (err) {
      console.log(err);
      process.exit(1);
    }

    callback(client, done);
  });
}

function createUser(callback) {
  'use strict';

  console.log('Creating user : ', envConfig.database.user);

  connect(function(client, done) {
    var query = {
      text: 'CREATE USER ' + envConfig.database.user + ' WITH ENCRYPTED PASSWORD \'' + envConfig.database.password + '\'',
    };

    client.query(query, function(err, result) {
      done();

      if (err) {
        console.log(err);
        return callback();
      }

      console.log('Created user : ', envConfig.database.user);
      callback();
    });
  });
}

function createDatabase(callback) {
  'use strict';

  console.log('Creating database : ', envConfig.database.database);

  connect(function(client, done) {
    var query = {
      text: 'CREATE DATABASE ' + envConfig.database.database + ' WITH OWNER = ' + envConfig.database.user
    };

    client.query(query, function(err, result) {
      done();

      if (err) {
        console.log(err);
        return callback();
      }

      console.log('Created database : ', envConfig.database.database);

      callback();
    });
  });
}

function createSchema(callback) {
  'use strict';

  console.log('Creating schema...');

  connect(function(client, done) {
    var file = fs.readFileSync(__dirname + '/../lib/stores/postgres/sql/setup.sql').toString();
    var query = {
      text: file,
    };

    client.query(query, function(err, result) {
      done();

      if (err) {
        console.log(err);
        return callback();
      }

      console.log('Schema created');
      callback();
    });
  });
}

module.exports = function() {
  'use strict';

  // use the postgres master uname/pass so we can create the user and DB
  currentConfig = JSON.parse(JSON.stringify(postgresUserConfig));
  createUser(function() {
    createDatabase(function() {

      // use the normal env specific config user/pass to do everything else.
      currentConfig = JSON.parse(JSON.stringify(envConfig));
      createSchema(function() {
        console.log('Done');
        process.exit();
      });
    });
  });
};
