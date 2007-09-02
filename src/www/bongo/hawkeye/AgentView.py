import bongo.commonweb
import bongo.commonweb.BongoUtil
from HawkeyeHandler import HawkeyeHandler

doneop = 0

class AgentHandler(HawkeyeHandler):

    def index_GET(self, req, rp):
        global doneop
        
        config = self.GetStoreData(req, "/config/manager")
        
        # change things which eval. to False to empty elements
        if config != None and config.has_key("agents"):
            for agent in config["agents"]:
               # We can change this if needed?
               agent["url"] = "agents/" + agent["name"]
               agent["rname"] = self._humanReadableName(agent["name"])
               agent["img"] = "../img/agent-" + agent["name"] + ".png"
               if not agent["enabled"]:
                   agent["enabled"] = None
            self.SetVariable("agentlist", config["agents"])
            self.SetVariable("success", 1)
            self.SetVariable("error", None)
        elif config != None:
            self.SetVariable("success", 0)
            self.SetVariable("error", "We managed to read the configuration from store, but you don't seem to have any agents setup.")
        else:
            self.SetVariable("success", 0)
            # Error is set for us via self.GetStoreData()
            
        if doneop:
            self.SetVariable("opsuccess", 1)
            doneop = 0
        else:
            self.SetVariable("opsuccess", 0)
        
        self.SetVariable("agntab", "selecteditem")
        return self.SendTemplate(req, rp, "index.tpl")
    
    def bongoantispam_GET(self, req, rp):
        # Antispam
        return self.doAgent("bongoantispam", self.GetStoreData(req, "/config/antispam"), req, rp)
        
    def bongomailprox_GET(self, req, rp):
        # Mail proxy
        return self.doAgent("bongomailprox", self.GetStoreData(req, "/config/mailproxy"), req, rp)    
    
    def bongoavirus_GET(self, req, rp):
        # Antivirus
        return self.doAgent("bongoavirus", self.GetStoreData(req, "/config/antivirus"), req, rp)
    
    def bongoqueue_GET(self, req, rp):
        # Queue
        return self.doAgent("bongoqueue", self.GetStoreData(req, "/config/queue"), req, rp)
    
    def bongopop3_GET(self, req, rp):
        # POP3
        return self.doAgent("bongopop3", self.GetStoreData(req, "/config/pop3"), req, rp)
    
    def bongoimap_GET(self, req, rp):
        # IMAP
        return self.doAgent("bongoimap", self.GetStoreData(req, "/config/imap"), req, rp)
    
    def bongosmtp_GET(self, req, rp):
        # SMTP
        return self.doAgent("bongosmtp", self.GetStoreData(req, "/config/smtp"), req, rp)
        
    def bongosmtp_POST(self, req, rp):
        # SMTP POST
        print str(rp)
        return bongo.commonweb.HTTP_UNAUTHORIZED
    
    def doAgent(self, agentname, config, req, rp):
        rkeys = []
        
        if config != None and config.has_key("version"):
            for key in config:
                # Don't allow block_rts_spam - unused.
                if key != "block_rts_spam":
                    nkey = {}
                    nkey["id"] = key
                    nkey["value"] = config[key]
                    
                    # Don't allow version as editable.
                    if key != "version":
                        nkey["name"] = self._humanReadableKey(key) + ":"
                        suffix = self.getParamSuffix(key)
                        if suffix:
                            nkey["suffix"] = suffix
                        nkey["numericentry"] = self.isNumericEntry(key)
                        if not nkey["numericentry"]:
                            nkey["boolentry"] = self.isBoolEntry(key)
                            if not nkey["boolentry"]:
                                # Has to be text entry if it ain't numeric or boolean.
                                nkey["textentry"] = True
                            else:
                                if config[key] == False:
                                    nkey["checked"] = None
                                else:
                                    nkey["checked"] = True
                    else:
                        nkey["hidden"] = True
                
                    rkeys.append(nkey)
                
            self.SetVariable("settinglist", rkeys)
        elif config != None:
            self.SetVariable("success", 0)
            self.SetVariable("error", "Agent does not have a version key.")
        else:
            self.SetVariable("success", 0)
        
        self.SetVariable("agntab", "selecteditem")
        self.SetVariable("name", agentname)
        self.SetVariable("hname", self._humanReadableName(agentname))
        return self.SendTemplate(req, rp, "agentview.tpl")
    
    def index_POST(self, req, rp):    
        global doneop
        
        if not req.form:
            return bongo.commonweb.HTTP_UNAUTHORIZED
        
        if not req.form.has_key("command"):
            return bongo.commonweb.HTTP_UNAUTHORIZED
        
        # Operation completed OK.
        doneop = 1
        return self.index_GET(req, rp)
    
    def isBoolEntry(self, k):
        keys = [ 'use_relay_host', 'allow_expn', 'block_rts_spam', 'allow_vrfy', 'allow_client_ssl', 'verify_address', 'relay_local_mail', 'send_etrn', 'accept_etrn' ]
        if keys.count(k) > 0:
            return True
        else:
            # Return None, not False (for Tal)
            return None
    
    def isNumericEntry(self, k):
        keys = [ 'port', 'port_ssl', 'max_mx_servers', 'socket_timeout', 'maximum_recipients', 'max_flood_count', 'message_size_limit', 'max_thread_load', 'max_null_sender', 'threads_max' ]
        
        if keys.count(k) > 0:
            # If it's in 'keys', return True.
            return True
        else:
            # Return None, not False (for Tal)
            return None
    
    def getParamSuffix(self, k):
        keys = { 'interval': 'seconds', 'socket_timeout':'seconds', 'message_size_limit': 'bytes (set to 0 for none)' }
        
        if keys.has_key(k):
            return keys[k]
        else:
            return None
    
    def _humanReadableKey(self, k):
        """
        Converts a setting key name into a human-readable keyname.
        """
        
        keys = { 'port': 'Port', 'port_ssl': 'SSL Port', 'hostname': 'Hostname', 'hostaddr': 'Host address', 'allow_client_ssl': 'Allow client SSL', 'allow_vrfy': 'Allow verification', 'relay_host': 'Relay to host', 'max_mx_servers': 'Maximum MX servers', 'socket_timeout': 'Socket timeout', 'block_rts_spam': 'Block RTS spam', 'verify_address': 'Verify address before accepting mail', 'relay_local_mail': 'Relay local mail', 'max_flood_count': 'Maximum flood count', 'message_size_limit': 'Maximum message size', 'maximum_recipients': 'Maximum recipients', 'use_relay_host': 'Use relay host', 'threads_max': 'Maximum threads', 'postmaster': 'Postmaster', 'max_threads': 'Maximum threads', 'interval': 'Interval', 'queue':'Queue host', 'max_accounts':'Maximum accounts', 'host':'Host', 'flags':'Operation mode(s) - see avirus.h', 'minimumfreespace':'Minimum free quota space', 'send_etrn':'Send ETRN', 'allow_expn':'Allow group expansion', 'accept_etrn':'Accept ETRN', 'max_thread_load':'Maximum thread load', 'max_null_sender':'Maximum recipients for null sender' }
        
        if keys.has_key(k):
            return keys[k]
        else:
            # Return the unchanged key back if we dunno
            # what to do with it.
            return k
    
    def _humanReadableName(self, agentname):
        # TODO: Translation, once we have a system in place.
        if agentname == "bongoqueue":
            return "Queue"
        elif agentname == "bongosmtp":
            return "SMTP"
        elif agentname == "bongoantispam":
            return "Antispam"
        elif agentname == "bongoavirus":
            return "Antivirus"
        elif agentname == "bongocollector":
            return "Calendar collector"
        elif agentname == "bongomailprox":
            return "Mail proxy"
        elif agentname == "bongopluspack":
            return "Plus pack"
        elif agentname == "bongorules":
            return "Rules"
        elif agentname == "bongoimap":
            return "IMAP"
        elif agentname == "bongopop3":
            return "POP3"
        elif agentname == "bongocalcmd":
            return "iCal"
        else:
            return "Unknown agent"
