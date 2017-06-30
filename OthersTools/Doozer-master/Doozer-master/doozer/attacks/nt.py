from core.generic_hash import GenericHash
import core.util as util
import core.sethor as sethor


class Hash(GenericHash):
    """ This object defines our NT hash type; expected format is
    pwdump
    """

    def __init__(self):
        super(Hash, self).__init__()
        self.type = "nt"

    def getRules(self):
        """ Return a list of rules for the NT hash type
        """

        hashcat = {
                      "wordlist mode" :
                          # wordlist mode
                          "{0} --hash-type 1000 --remove --disable-potfile"\
                          " --disable-restore --session {1} -o {2}/nt.cracked "\
                          "{2}/nt.list {3}".format(sethor.HASHCAT_BINARY,
                                                   self.session,
                                                   self.session_home,
                                                   sethor.WORDLIST_DIR),
                      "wordlist with rules" :
                          # wordlist with rules
                          "{0} --hash-type 1000 --remove --disable-potfile --disable-restore"\
                          " -r {2}/rules/toggles1.rule -r {2}/rules/toggles2.rule"\
                          " -r {2}/rules/toggles3.rule -r {2}/rules/toggles4.rule"\
                          " -r {2}/rules/toggles5.rule -r {2}/rules/passwordspro.rule"\
                          " --session {1} -o {3}/nt.cracked {3}/nt.list {4}".format(
                                          sethor.HASHCAT_BINARY,
                                          self.session,
                                          sethor.HASHCAT_DIR,
                                          self.session_home,
                                          sethor.WORDLIST_DIR)
                    }

        # pattern mode; build separate rules for each pattern that we support
        patterns = ["?s?u?l?l?l?l?l?l", "?d?d?u?l?l?l?l?l", "?d?u?l?l?l?l?l?l",
                    "?u?l?l?l?l?l?d?s", "?u?l?l?l?l?l?d?d", "?u?l?l?l?l?l?l?d?d",
                    "?u?l?l?l?l?d?d?d?d"]

        for (idx, pattern) in enumerate(patterns):
    
            hashcat["pattern %d attack"%idx] = "{0} --hash-type 1000 --remove "\
                                               "--disable-potfile --disable-restore"\
                " --session {1} -o {2}/nt.cracked {2}/nt.list -a 3 {3}".format(
                           sethor.HASHCAT_BINARY,
                           self.session,
                           self.session_home,
                           pattern)

        return hashcat

    def check(self, value, initial_check=True):
        """ Validate a hash value as a pwdump NT hash; if initial_check is
        not True, then we are preparing to crack hashes.  this will add them
        to an internal data structure.
        """
        
        bad_values = ["ASPNET", "IUSR_", "\\\\$"]
        valid = True

        if not initial_check:
            # check if it's a bad value
            for bad in bad_values:
                if bad in value:
                    valid = False 

        # check if its malformed
        if valid:
            try:
                (_, nt) = value.split(':')[2:4]
                if len(nt) != 32:
                    valid = False
            except:
                valid = False

        # check if we've already popped it
        if valid and not initial_check and nt not in self.cracked_hashes:
            tmp = util.check_doozer(nt, self.type)
            if tmp:
                self.cracked(nt, tmp)
                valid = False
       
        # check header
        (match, ctype) = self.checktype()
        if valid and ((match is False) and (not ctype is None)):
            # special case for pwdump files; no type spec
            # means default to NT
            valid = False

        # if it's valid  
        if not initial_check and valid:
            self.hashes.append(nt)
            self.clean_hash += 1    

        if not initial_check:
            self.start_hash += 1

        return valid

    def parseCracked(self, line):
        """ Parse out hashcat's NT hash
        """

        (hsh, pswd) = line.split(':')
        return (hsh.translate(None, '\n'), pswd.translate(None, '\n'))
