/* Crypton Server, Copyright 2014 SpiderOak, Inc.
 *
 * This file is part of Crypton test suite.
 *
 * Crypton is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton.  If not, see <http://www.gnu.org/licenses/>.
*/

describe('Change Passphrase', function () {
  this.timeout(200000);
  describe('Account generation', function () {

    var _testUICallback = false;
    var initialSession;
    var options = { check: true };
    var CONTAINER_NAME = 'my-fuzzy-container';

    it('generate account, save container', function (done) {
      crypton.generateAccount('drevil', 'password', function (err, account) {
        assert.equal(err, null);
        if (err) {
          done();
        }

        // Authorize
        crypton.authorize('drevil', 'password', function (err, session) {
          assert.equal(err, null);
          if (err) {
            console.error('Auth: ', err);
            done();
          }
          initialSession = session;
          // Let's crete a container
          session.create(CONTAINER_NAME, function (err, container) {
            assert.equal(err, null);
            if (err) {
              console.error('Create: ', err);
              done();
            }
            // add data to the container
            container.add('Orb', function (err) {
              assert.equal(err, null);
              if (err) {
                console.error('Add: ', err);
                done();
              }
              container.keys['Orb'].sound = 'HUGE';
              container.save(function () {
                console.log('Container save callback, no error handong here, just console.error statements if transactions fail');
                // XXXddahl: There is no error handling here yet, see client/src/container.js [line 137]
                done();
              });
            });

          });
        }); // end auth
      }); // end generateAccount
    }); // end it()

    it('Change passphrase without error', function (done) {
      function cb (err, isComplete) {
        assert.equal(err, null);
        if (err) {
          console.error('change pass callback: ', err);
          done();
        }
        assert.equal(_testUICallback, true);
        assert.equal(isComplete, true);
        done();
      }

      function uiProgressCallback () {
        _testUICallback = true;
      }

      try {
        initialSession.account.changePassphrase('password', 'foobarstrongerpass', cb, uiProgressCallback, false);
      } catch (ex) {
        console.error(ex);
        initialSession = null;
        done();
      }
    });

    it('test changed passphrase, check container', function (done) {
      crypton.authorize('drevil', 'foobarstrongerpass', function (err, newSession) {
        if (err) {
          console.error(err);
          done();
        }
        assert.equal(newSession.account.username, 'drevil');
        // Lets check the content of our container
        newSession.load(CONTAINER_NAME, function (err, container) {
          assert(container);
          assert.equal(container.keys['Orb'].sound, 'HUGE');
          done();
        });

      }); // end 2nd auth
    }); // end it()
  });
});
