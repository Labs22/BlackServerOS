/* Example SQL code showing committing a transaction on an unsharded database,
 * using just SQL commands. */

/* run this by doing:
    make clean-test-db
    make setup-test-environment
    sudo -u postgres psql -f server/lib/stores/postgres/sample_transaction.sql
*/

/* give us readable, long form output from all of the test selects */
\x                        

/* first we build up a test account, and a dummy transaction, so we cant test
 * the transaction being applied. */


begin;
/* CREATE TEST ACCOUNT */

insert into account (username, base_keyring_id) values(
    'test_username', (select nextval('version_identifier')));

insert into base_keyring (
    base_keyring_id,
    account_id, 
    srp_verifier, 
    srp_salt,
    keypair_salt, 
    keypair_iv, 
    keypair, 
    pubkey, 
    container_name_hmac_key_iv, 
    container_name_hmac_key, 
    hmac_key_iv, 
    hmac_key) values (
    (select base_keyring_id from account where username='test_username'),
    (select account_id from account where username='test_username'),
    'srp_verifier23456789012345678901',
    'srp_salt000123456789012345678901',
    'keypair_salt23456789012345678901',
    'keypair_iv012345',
    'keypair_fake2345',
    'pubkey_fake',
    'name_hmac_key_iv',
    'container_name_hmac_key345678901',
    'hmac_key_iv12345',
    'hmac_key890123456789012345678901');

commit;

/* CREATE TEST TRANSACTION */

begin;
insert into transaction (account_id) 
    select account_id from account where username='test_username';
   
insert into transaction_add_container (transaction_id, name_hmac)
    select transaction_id, 'container_name_hmac_012345678901'
      from transaction
     where account_id=(select account_id 
                         from account 
                        where username='test_username');
   
insert into transaction_add_container_session_key (
    transaction_id, name_hmac, signature )
    select transaction_id, 'container_name_hmac_012345678901', 
           'TODO_PRIVATE_KEY_SIGNATURE_FAKE'
      from transaction
     where account_id=(select account_id 
                         from account 
                        where username='test_username');

insert into transaction_add_container_session_key_share
    ( transaction_id, name_hmac, to_account_id, 
      session_key_ciphertext, hmac_key_ciphertext )
    select transaction_id, 'container_name_hmac_012345678901', account_id,
           'session_key_ciphertext_fake', 'hmac_key_ciphertext_fake'
      from transaction
     where account_id=(select account_id 
                         from account 
                        where username='test_username');

/* TODO: resolve the race condition with 
 * container_record.container_session_key_id 
 */

insert into transaction_add_container_record
    ( transaction_id, name_hmac, hmac, payload_iv, payload_ciphertext )
    select transaction_id, 
           'container_name_hmac_012345678901',
           'container_record_hmac_2345678901',
           'payload_iv_12345',
           'payload_ciphertext_9012345678901'
      from transaction
     where account_id=(select account_id 
                         from account 
                        where username='test_username');

commit;

/* REQUEST THE TRANSACTION TO BE COMMITTED */

begin;
update transaction 
   set commit_request_time = current_timestamp 
 where account_id=(select account_id 
                     from account 
                    where username='test_username');
commit;


/* MARK THAT WE ARE WORKING ON COMMITTING THE TRANSACTION */
begin;
update transaction 
   set commit_start_time = current_timestamp,
       committer_hostname = 'test_committer_host'
 where account_id=(select account_id 
                     from account 
                    where username='test_username');
commit;

/* COMMIT THE TRANSACTION: PART 1: PRECALCULATION */

/* we make temp versions of each of the transaction tables, which we can join
 * with the original tables, and add any extra columns we need to calculate.
 */

begin;

create temp table txtmp_add_container as 
    select tac.id,
           tac.name_hmac,
           nextval('version_identifier') as container_id,
           t.account_id,
           /* we'll update latest_record_id below */
           null::int8 as latest_record_id 
      from transaction t
      join transaction_add_container tac using (transaction_id)
     where account_id=(select account_id 
                         from account 
                        where username='test_username');
select * from transaction_add_container join txtmp_add_container using (id);

/* create temp table for transaction_add_container_session_key
 * that adds new columns: container_session_key_id
 *                        container_id,
 */
create temp table txtmp_add_container_session_key as
    select tacsk.id,
           nextval('version_identifier') as container_session_key_id,
           /* there are two possibilities for the container_id when adding a
            * session key: 
            *   a new container added in this transaction
            *   an existing container
            * check for the new container case first, fallback to existing
            * container
            */
           coalesce(
               (select container_id 
                  from txtmp_add_container
                 where name_hmac=tacsk.name_hmac),
               (select container_id
                  from container
                 where name_hmac=tacsk.name_hmac)
           ) as container_id,
           name_hmac
      from transaction_add_container_session_key tacsk;

select tacsk.*, tx_tacsk.*, transaction.*
  from transaction_add_container_session_key tacsk
  join txtmp_add_container_session_key tx_tacsk using (id)
  join transaction using (transaction_id);

/* calculate new columns: container_session_key_id */
create temp table txtmp_add_container_session_key_share as
    select tacsks.id,
           nextval('version_identifier') as container_session_key_share_id,
           /* once again, two possibilities */
           coalesce(
               (select container_session_key_id
                  from txtmp_add_container_session_key
                 where name_hmac=tacsks.name_hmac),
               (select container_session_key_id
                  from container_session_key
                 where container_id=(select container_id
                                       from container
                                      where name_hmac=tacsks.name_hmac))
           ) as container_session_key_id
      from transaction_add_container_session_key_share tacsks;

select tacsks.*, tx_tacsks.*, transaction.*
  from transaction_add_container_session_key_share tacsks
  join txtmp_add_container_session_key_share tx_tacsks using (id)
  join transaction using (transaction_id);

/* calculate new columns: 
 *  container_record_id, container_id, container_session_key_id */

create temp table txtmp_add_container_record as
    select tar.id,
           nextval('version_identifier') as container_record_id,
           coalesce(
               (select container_id 
                  from txtmp_add_container
                 where name_hmac=tar.name_hmac),
               (select container_id
                  from container
                 where name_hmac=tar.name_hmac)
           ) as container_id,
           coalesce(
               (select container_session_key_id
                  from txtmp_add_container_session_key
                 where name_hmac=tar.name_hmac),
               (select container_session_key_id
                  from container_session_key
                 where container_id=(select container_id
                                       from container
                                      where name_hmac=tar.name_hmac))
           ) as container_session_key_id
  from transaction_add_container_record tar;

select tacr.*, tx_tacr.*, t.*
  from transaction_add_container_record tacr
  join txtmp_add_container_record tx_tacr using (id)
  join transaction t using (transaction_id);

/* now, we can finally calculate the latest_record_id value for new containers
 * we're adding */
update txtmp_add_container set latest_record_id=(
    select max(container_record_id) 
      from txtmp_add_container_record
     where txtmp_add_container_record.container_id = 
           txtmp_add_container.container_id);

/* COMMITTING THE TRANSACTION: PART 2: MODIFYING PRIMARY TABLES */

/* now we've calculated all the intermediate values we need, and we update the
 * primary tables. */

insert into container (container_id, account_id, name_hmac, latest_record_id)
    select tx_tac.container_id, t.account_id, tac.name_hmac, tx_tac.latest_record_id
      from transaction_add_container tac
      join txtmp_add_container tx_tac using (id)
      join transaction t using (transaction_id);

insert into container_session_key (container_session_key_id, container_id, 
    account_id, transaction_id, signature)
    select tx_tacsk.container_session_key_id, tx_tacsk.container_id,
           t.account_id, t.transaction_id, tacsk.signature
      from transaction_add_container_session_key tacsk
      join txtmp_add_container_session_key tx_tacsk using (id)
      join transaction t using (transaction_id);

insert into container_session_key_share (container_session_key_share_id, 
    container_session_key_id, account_id, to_account_id, 
    transaction_id, session_key_ciphertext, 
    hmac_key_ciphertext)
    select tx_tacsks.container_session_key_share_id,
           tx_tacsks.container_session_key_id, 
           t.account_id, tacsks.to_account_id,
           t.transaction_id, tacsks.session_key_ciphertext, 
           tacsks.hmac_key_ciphertext
      from transaction_add_container_session_key_share tacsks
      join txtmp_add_container_session_key_share tx_tacsks using (id)
      join transaction t using (transaction_id);

insert into container_record (container_record_id, container_id, 
    container_session_key_id, account_id, transaction_id, 
    hmac, payload_iv, payload_ciphertext)
    select tx_tacr.container_record_id, tx_tacr.container_id,
           tx_tacr.container_session_key_id, t.account_id, t.transaction_id,
           tacr.hmac, tacr.payload_iv, tacr.payload_ciphertext
      from transaction_add_container_record tacr
      join txtmp_add_container_record tx_tacr using (id)
      join transaction t using (transaction_id);

commit;

/* OK, that is applying the simplest case for a transaction, that just adds a
 * new container, with a session key, shared to the account owner, with one
 * record.

 * The same code may work for updates to existing containers, and shares of
 * existing containers, but no test for that yet so not sure.
*/

/* TODO:
    mark the transaction as committed in the transaction table
    apply supercede_time to previous session keys when adding new session keys
    schema change: add transaction_id to container
    schema change: add container_session_key_id to container_record to resolve
        race condition mentioned above
    apply deletes to container_session_key_share (when containers are unshared with some users)
    apply deletes to containers
    apply messages
 */

