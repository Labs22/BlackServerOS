from core.generic_hash import GenericHash
import core.util as util
import core.sethor as sethor
import os

class Hash(GenericHash):

    def __init__(self):
        super(Hash, self).__init__()
        self.type = 'ntlmv2'

    def getRules(self):
        """ Return a dictionary of our NTLMv2 rules
        """

        rules = { 
                    "wordlist mode" :
                        "{0} --hash-type 5600 --remove --disable-potfile"\
                        " --disable-restore --session {1} -o {2}/ntlmv2.cracked "\
                        "{2}/ntlmv2.list {3}".format(sethor.HASHCAT_BINARY,
                                                     self.session,
                                                     self.session_home,
                                                     sethor.WORDLIST_DIR)
                }

        return rules

    def check(self, value, initial_check=True):
        """
        """

        valid = True

        # check if it's malformed
        try:
            ntlm = value.split(':')[4]
            if len(ntlm) != 32:
                valid = false
        except:
            valid = False

        # check header
        if valid and not self.checktype()[0]:
            valid = False

        if not initial_check and valid and ntlm not in self.cracked_hashes:
            tmp = util.check_doozer(ntlm, self.type)
            if tmp:
                self.cracked(ntlm, tmp)
                valid = False

        if not initial_check and valid:
            # write out the entire line, because we need it
            util.msg("Appending hash %s" % value.upper())
            self.hashes.append(value.upper())
            self.clean_hash += 1

        if not initial_check:
            self.start_hash += 1

        return valid
 
    def parseCracked(self, line):
        """ We'll return the actual hash + password
        """

        hsh = line.split(':')[4].translate(None, '\n')
        pswd = line.split(':')[6].translate(None, '\n')
        return (hsh, pswd)

    def run(self):
        """ Override default behavior because the way we crack and store NTLMv2 hashes
        is different than the base case.
        """

        rules = self.getRules()

        for rule in rules:

            # write out current to-crack hashes
            self.dump()

            try:
                util.msg("Running %s" % rule)
                os.system(rules[rule])
            except Exception, e:
                util.msg("rule '%s' failed: %s" % (rule, e))
                continue

            if os.path.exists("%s/%s.cracked" % (self.session_home, self.type)):

                with open("%s/%s.cracked" % (self.session_home, self.type)) as f:
                    
                    for line in f.readlines():
                        try:
                            (hsh, p) = self.parseCracked(line)
                            fdata = "%s:%s:%s" % (hsh, p, self.type)
                            util.msg("Created fdata %s from line %s" % (fdata, line)) 
                            # remove hash from local struct; hashcat appends the pwd
                            # to the end, so we need to rebuild
                            self.hashes.remove(':'.join(line.split(':')[:-1]).upper())
                            self.cracked_hashes.append(fdata)
                        except Exception, e:
                            util.msg("Failed to format cracked line '%s': %s" % (line, e))

                os.system("rm %s/%s.cracked" % (self.session_home, self.type))
            os.system("rm %s/%s.list" % (self.session_home, self.type))
        self.writePotFile(self.cracked_hashes)
