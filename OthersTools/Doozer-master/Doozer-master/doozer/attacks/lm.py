from core.generic_hash import GenericHash
import core.util as util
import core.sethor as sethor

class Hash(GenericHash):
    
    def __init__(self):
        super(Hash, self).__init__()
        self.type = "lm"        

    def getRules(self):
        """
        """

        rules = {
                    # ophcrack tables
                    "ophcrack lm tables":
                            "ophcrack -g -n 5 -e -d {0} -t XP_special -f {1}/lm.list"\
                            " -S {1}/ophcrack.session -o {1}/lm.cracked".format(
                                            sethor.OPHCRACK_TABLES,
                                            self.session_home
                                            )
                }

        return rules

    def check(self, value, initial_check=True):
        """
        """

        bad_values = ["NO PASSWORD****", "aad3b435b51404eeaad3b435b51404ee",
                      "00000000000000000000000000000000"
                     ]
        valid = True

        if not initial_check:
            # check for bad values only when it's not a dry run check
            for bad in bad_values:
                if bad in value:
                    valid = False

        # check if it's malformed
        if valid:
            try:
                (lm, _) = value.split(':')[2:4]
                if len(lm) != 32:
                    valid = False
            except:
                valid = False

        # check if we've already popped it
        if valid and not initial_check:
            tmp = util.check_doozer(lm, self.type)
            if tmp:
                self.cracked(lm, tmp)
                valid = False

        # check header
        if valid and not self.checktype()[0]:
            valid = False

        # if it's valid
        if not initial_check and valid:
            self.hashes.append(lm)
            self.clean_hash += 1

        if not initial_check:
            self.start_hash += 1

        return valid

    def parseCracked(self, line):
        """ parse out the ophcrack LM hash
        """

        hash = line.split(":")[2]
        plaintext = ''.join(line.split(':')[4:]).translate(None, '\n')
        return (hash, plaintext)
