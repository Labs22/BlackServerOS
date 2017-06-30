
begin;
update transaction
   set commit_start_time = current_timestamp,
       committer_hostname = '{{hostname}}'
 where transaction_id={{transactionId}};
commit;

/* COMMIT THE TRANSACTION: PART 1: PRECALCULATION */

/* we make temp versions of each of the transaction tables, which we can join
 * with the original tables, and add any extra columns we need to calculate.
 */
begin;

create temp table txtmp_add_container on commit drop as
    select tac.id,
           tac.name_hmac,
           nextval('version_identifier') as container_id,
           t.account_id,
           /* we'll update latest_record_id below */
           null::int8 as latest_record_id 
      from transaction t
      join transaction_add_container tac using (transaction_id)
     where transaction_id={{transactionId}};

/*
     where account_id=(select account_id 
                         from account 
                        where username='test_username');
*/
/* create temp table for transaction_add_container_session_key
 * that adds new columns: container_session_key_id
 *                        container_id,
 */
create temp table txtmp_add_container_session_key on commit drop as
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
      from transaction_add_container_session_key tacsk
     where transaction_id={{transactionId}};

/* calculate new columns: container_session_key_id */
create temp table txtmp_add_container_session_key_share on commit drop as
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
      from transaction_add_container_session_key_share tacsks
     where transaction_id={{transactionId}};

/* calculate new columns: 
 *  container_record_id, container_id, container_session_key_id */

create temp table txtmp_add_container_record on commit drop as
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
  from transaction_add_container_record tar
 where transaction_id={{transactionId}};

create temp table txtmp_delete_container on commit drop as
    select container_id
      from container
     where name_hmac = (
       select tdc.name_hmac
         from transaction_delete_container tdc
        where tdc.transaction_id = {{transactionId}}
     );

create temp table txtmp_delete_message on commit drop as
    select message_id
      from transaction_delete_message
     where transaction_id = {{transactionId}};

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
    transaction_id, session_key_ciphertext)
    select tx_tacsks.container_session_key_share_id,
           tx_tacsks.container_session_key_id, 
           t.account_id, tacsks.to_account_id,
           t.transaction_id, tacsks.session_key_ciphertext
      from transaction_add_container_session_key_share tacsks
      join txtmp_add_container_session_key_share tx_tacsks using (id)
      join transaction t using (transaction_id);

insert into container_record (container_record_id, container_id, 
    container_session_key_id, account_id, transaction_id,  payload_ciphertext)
    select tx_tacr.container_record_id, tx_tacr.container_id,
           tx_tacr.container_session_key_id, t.account_id,
           t.transaction_id, tacr.payload_ciphertext
      from transaction_add_container_record tacr
      join txtmp_add_container_record tx_tacr using (id)
      join transaction t using (transaction_id);

update container
  set deletion_time = current_timestamp
  where container_id in (
    select container_id from txtmp_delete_container
  );

update message
  set deletion_time = current_timestamp
  where message_id in (
    select message_id from txtmp_delete_message
  );

update transaction
   set commit_finish_time = current_timestamp,
       success = true,
       committer_hostname = '{{hostname}}'
 where transaction_id={{transactionId}};

/*
 * after all other operations have had the chance to err out,
 * notify any clients about container record insertions that may have happened
 *
 * TODO is there a more eloquent way to structure this query?
 */
select pg_notify('container_update', encode(name_hmac, 'escape') || ':' || csks.account_id::text || ':' || csks.to_account_id::text)
  from txtmp_add_container_record tx_tacr
  join container using (container_id)
  join container_session_key_share csks using (container_session_key_id);

commit;
