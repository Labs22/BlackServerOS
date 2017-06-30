Deployment Automation
=====================

We use ansible to automate configuring a machine (or many machines) to serve
Crypton.  A similar system Ansible configuration (with only different
variables) is used for dev and production.  

This arranges things like installing nginx, postgresql, redis, nodejs, the
crypton server, placing certificates, arranging run scripts, usix users,
database users and passwords, database ownership and permissions, nginx site
config files, log rotation, ad nauseam...

To see how this works, start by reading the inventory of hosts/groups:
`hosts.dev`.  Then read the ansible playbook `site.yml` to see the plays and
roles applied.  For each role, start by reading `roles/NAME/tasks/tagged.yml`.

Bootstrapping a Dev Simulation Environment
==========================================

To quickly setup a dev environment, there's a bootstrap script that
installs/configures all dependencies needed on the host, creates an LXC
guest OS container running Ubuntu 12.04 LTS, and finally uses ansible to
automate configuring that container machine to serve crypton.  The IP for the
guest (hostname `crypton1`) will be added to `/etc/hosts` on the host.  This
has been tested using Ubuntu 13.10 hosts.

```
scripts/bootstrap-dev.sh
```

If you prefer not to use LXC or an Ubuntu host, you can arrange for your own
Ubuntu 12.04 (virtual or metal) machine to be running somewhere, and then use
the same ansible commands found in the script above to configure your machine
for Crypton.  In that case you may wish to edit the inventory of hosts
(`hosts.dev`) to specify a different host, and the `ansible.cfg` to specify a
different `remote_user` for ssh.


Connecting Your Browser to the Crypton Dev Server
=================================================

The scripts above work with the domain `cryptondev.local`.  They generate self
signed wildcard SSL certificates, and nginx accepts connections on local IPs,
with server name `server.cryptondev.local`.

Ansible (by applying the `dev_proxy` role) also installs and configures a squid
http proxy server on the crypton server machine, listening on port `3128`.  If
you configure your browser to use this proxy, you'll be able to browse to URLs
like `https://server.cryptondev.local/examples/diary/` (after accepting the SSL
certificate.)  

Note that the proxy is configured to ONLY allow connections to the local
machine, ports 80 and 443.  So if a cryptondev.local site makes references to
URLs not served by that machine (such as Google fonts, jquery, etc.) the proxy
will disallow those and the page won't work correctly.  This will remind you
not to make create references to 3rd party javascript anyway. :)

Sometimes it is helpful to use multiple browsers running with different
profiles.
