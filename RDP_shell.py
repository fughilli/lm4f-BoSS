import sys

def split_on_first(toks, sep):
    try:
        sepidx = toks.index(sep)
        return (True,toks[:sepidx], toks[sepidx+1:])
    except Exception:
        return (False,toks,[])
        

def parse_pipe(pipetoks):
    ssucc,lside,rside = split_on_first(pipetoks, '|')
    if(ssucc):
        lsucc, lsideremtoks = parse_token_string(lside)
        rsucc, rsideremtoks = parse_token_string(rside)
        if(lsucc and rsucc and (len(lsideremtoks) == 0)):
            return (("Pipe",lsucc,rsucc), rsideremtoks)
    return (None, pipetoks)

def parse_conditional(condtoks):
    ssucc,lside,rside = split_on_first(condtoks, '&&')
    if(ssucc):
        lsucc, lsideremtoks = parse_token_string(lside)
        rsucc, rsideremtoks = parse_token_string(rside)
        if(lsucc and rsucc and (len(lsideremtoks) == 0)):
            return (("And",lsucc,rsucc), rsideremtoks)
    ssucc,lside,rside = split_on_first(condtoks, '||')
    if(ssucc):
        lsucc, lsideremtoks = parse_token_string(lside)
        rsucc, rsideremtoks = parse_token_string(rside)
        if(lsucc and rsucc and (len(lsideremtoks) == 0)):
            return (("Or",lsucc,rsucc), rsideremtoks)
    return (None, condtoks)

def parse_redirect(redtoks):
    ssucc,lside,rside = split_on_first(redtoks, '>')
    if(ssucc):
        lsucc, lsideremtoks = parse_token_string(lside)
        rsucc, rsideremtoks = parse_fname(rside)
        if(lsucc and rsucc and (len(lsideremtoks) == 0)):
            return (("Redirect",lsucc,rsucc), rsideremtoks)
    ssucc,lside,rside = split_on_first(redtoks, '<')
    if(ssucc):
        lsucc, lsideremtoks = parse_token_string(lside)
        rsucc, rsideremtoks = parse_fname(rside)
        if(lsucc and rsucc and (len(lsideremtoks) == 0)):
            return (("Redirect",lsucc,rsucc), rsideremtoks)
    return (None, redtoks)

def parse_command(commandtoks):
    return (("Command",commandtoks), [])

def parse_fname(fnametoks):
    if(len(fnametoks) == 1):
        return (("Filename",fnametoks), [])
    else:
        return (None, [])

def parse_token_string(tokens):
    if(len(tokens) == 0):
        return (None, [])
    res, remtokens = parse_conditional(tokens)
    if(res):
        return res, remtokens
    res, remtokens = parse_pipe(tokens)
    if(res):
        return res, remtokens
    res, remtokens = parse_redirect(tokens)
    if(res):
        return res, remtokens
    return parse_command(tokens)
    
        
        

 

def parse_line(line):
    lintoks = line.split()
    ssucc,lside,rside = split_on_first(lintoks, "&")
    if(ssucc):
        if(len(rside) == 0):
            print "Run in background:"
        else:
            print "Unexpected &"
            return
    parsetree, remtoks = parse_token_string(lside)

    print parsetree
    print remtoks
    

while(1):
    parse_line(sys.stdin.readline())
