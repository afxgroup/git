/* OpenGuide.rexx */
OPTIONS RESULTS
PARSE ARG node

IF ~show('P', 'MV.GIT') THEN DO
    cmd = "Run >NIL: MultiView PORTNAME=MV.GIT Git:git-commands.guide"
    ADDRESS COMMAND cmd
    ADDRESS COMMAND 'WaitForPort' 'MV.GIT.1'
    IF RC ~= 0 THEN DO
        EXIT 10
    END
END
cmd = 'LINK ' || node
ADDRESS 'MV.GIT.1' cmd
ADDRESS 'MV.GIT' 'MAXIMUMSIZE'
