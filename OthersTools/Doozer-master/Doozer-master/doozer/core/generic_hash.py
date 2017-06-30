import core.sethor as sethor
import core.util as util
import os

class GenericHash(object):

    def __init__(self):
        self.type = None
        self.hashes = []
        self.cracked_hashes = []    # hash:plaintext:type

        self.session = None                                     # set by doozer 
        self.session_home = "%s/%s" % (sethor.WORKING_DIR, "")  # set by doozer 
        self.hash_file = None                                   # set by doozer 

        # statistics 
        self.start_hash = 0
        self.clean_hash = 0
        self.cracked_hash = 0

    def check(self, value, intial_check=True):
        """ 
        """

        raise NotImplementedError

    def getRules(self):
        """ Expected to return a dictionary of {string:string}, where the key is
        the name of the rule and the value is the actual rule.  The Rule should
        be an executable string. 
        """

        raise NotImplementedError

    def dump(self):
        """ Write out the hash data struct to the list file for cracking
        """
        
        with open("%s/%s.list" % (self.session_home, self.type), "w") as f:
            f.write('\n'.join(self.hashes))

    def cracked(self, hash, plaintext):
        """ Format a cracked hash and insert it into the pot file
        """
        
        fmt = "%s:%s:%s" % (hash, plaintext, self.type)
        self.cracked_hashes.append(fmt)

        #purge dupes
        self.cracked_hashes = list(set(self.cracked_hashes))
#        self.writePotFile([fmt])

    def checktype(self):
        """ We can prefix hash files with a hash type; check the
        type against the instance of the attack type.

        Returns a tuple of (bool, string) where String is the
        parsed type.
        """
        
        match = False
        ctype = None
        if '_' in self.hash_file:
            # we get a full path, so parse off the last entry
            ctype = self.hash_file.split('/', -1)[-1]
            # now parse type
            ctype = ctype.split('_', 1)[0]

            # verify the type is crackable
            attacks = os.listdir("attacks")
            if ('%s.py'%ctype) not in attacks:
                match = False
                ctype = None

            if ctype == self.type:
                match = True

        return (match, ctype)

    def writePotFile(self, values):
        """ Input needs to be a list
        """

        with open("%s/%s.pot" % (self.session_home, self.type), "a+") as f:
            for v in values:
                f.write(v + '\n')

    def parseCracked(self, line):
        """ Different applications will write out cracked hashes differently;
        this method will allow various attacks the ability to return the
        hash and password using their own parsing.

        Returned value should be a tuple of (hash, password)
        """

        raise NotImplementedError

    def run(self):
        """
        """

        rules = self.getRules()

        for rule in rules:

            # write out our current to-crack hashes
            self.dump()

            # run the rule
            try:
                util.msg("Running %s" % rule)
                os.system(rules[rule])
            except Exception, e:
                util.msg("rule '%s' failed: %s" % (rule, e)) 
                continue

            # check if we cracked any and, if we did, remove them from our
            # list of hashes and update the pot file
            if os.path.exists("%s/%s.cracked" % (self.session_home, self.type)):
                
                with open("%s/%s.cracked" % ( self.session_home, self.type)) as f:

                    lines = []
                    for line in f.readlines():
                        try:
                            # parse the cracked line out as per attack spec
                            (hsh, p) = self.parseCracked(line)
                            fdata = '%s:%s:%s' % (hsh, p, self.type)

                            # remove hash from local structure and add to cracked list
                            lines.append(fdata)
                            self.hashes.remove(hsh)
                            self.cracked_hashes.append(fdata)
                        except:
                            pass

                os.system("rm %s/%s.cracked" % (self.session_home, self.type))
            os.system("rm %s/%s.list" % (self.session_home, self.type))
        self.writePotFile(self.cracked_hashes) 
