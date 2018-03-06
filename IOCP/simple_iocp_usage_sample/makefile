TARGETNAME=dlgwin32
OUTDIR=.\chk

ALL : "$(OUTDIR)\$(TARGETNAME).exe"


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# compile
CPP_PROJ=\
	/MT\
	/W3\
	/Fo"$(OUTDIR)\\"\
	/Fd"$(OUTDIR)\\"\
	/c\
	/Zi

#resource compiler

RSC_PROJ=/l 0x411 /fo"$(OUTDIR)\$(TARGETNAME).res"

#link
LINK32=link.exe

LINK32_FLAGS=\
	user32.lib\
	/subsystem:windows\
	/pdb:"$(OUTDIR)\$(TARGETNAME).pdb"\
	/machine:I386\
	/out:"$(OUTDIR)\$(TARGETNAME).exe"\
	/DEBUG\
	/RELEASE
	
LINK32_OBJS= \
	"$(OUTDIR)\$(TARGETNAME).obj"\
	"$(OUTDIR)\$(TARGETNAME).res"\
	"$(OUTDIR)\debug.obj"

"$(OUTDIR)\$(TARGETNAME).exe" : "$(OUTDIR)" $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)

.c{$(OUTDIR)}.obj:
   $(CPP) $(CPP_PROJ) $< 

.cpp{$(OUTDIR)}.obj:
   $(CPP) $(CPP_PROJ) $< 

.rc{$(OUTDIR)}.res:
   rc.exe $(RSC_PROJ) $<