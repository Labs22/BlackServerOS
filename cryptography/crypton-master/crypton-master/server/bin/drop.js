var pg = require('pg').native;
var fs = require('fs');

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

  console.log('Using DB config : ', postgresUserConfig.database);

  // For DROP functions, always connecting as the master postgres user.
  pg.connect(postgresUserConfig.database, function(err, client, done) {
    if (err) {
      console.log(err);
      process.exit(1);
    }

    callback(client, done);
  });
}

function dropDatabase(callback) {
  'use strict';

  console.log('Dropping database : ', envConfig.database.database);

  connect(function(client, done) {
    var query = {
      text: 'DROP DATABASE ' + envConfig.database.database,
    };

    client.query(query, function(err, result) {
      done();

      if (err) {
        console.log(err);
        return callback();
      }

      console.log('Dropped database : ', envConfig.database.database);
      callback();
    });
  });
}

function dropUser(callback) {
  'use strict';

  console.log('Dropping user : ', envConfig.database.user);

  connect(function(client, done) {
    var query = {
      text: 'DROP ROLE ' + envConfig.database.user,
    };

    client.query(query, function(err, result) {
      done();

      if (err) {
        console.log(err);
        return callback();
      }

      console.log('Dropped user : ', envConfig.database.user);
      callback();
    });
  });
}

module.exports = function() {
  'use strict';

  dropDatabase(function() {
    dropUser(function() {
      console.log('Done dropping database and user.');
      process.exit();
    });
  });
};
