#  Makefile to construct the midaswww library
#  Tony Johnson, Oct, 1992


.First
	@ delete == "delete/noconf"
	@ define rpc     multinet_root:[multinet.include.rpc]
	@ define netinet multinet_root:[multinet.include.netinet]
	@ define sys     multinet_root:[multinet.include.sys]

NAME=midaswww


LIBRARY = $(NAME).olb

LIBMEMS = $(LIBRARY)(-
          custom.obj              , - 
          midas.obj               , - 
          midasclass.obj          , - 
          midasconvert.obj        , - 
          midasdeclare.obj        , - 
          midasevaluate.obj       , - 
          midasingot.obj          , - 
          midaslist.obj           , - 
          midasloadimage.obj      , - 
          midasutil.obj           , - 
          sgmlanchortext.obj      , - 
          sgmlcompositetext.obj   , - 
          sgmlformattedtext.obj   , - 
          sgmlhyper.obj           , - 
          sgmllisttext.obj        , - 
          sgmlmarkertext.obj      , - 
          sgmlplaintext.obj       , - 
          sgmltext.obj            , - 
          tree.obj)  
 
# Targets

all   : exe midas.uid midaswww.uid midas.dat midaswww.dat
	! done

midas.uid	: midas.uil
	UIL/MOTIF $<

midaswww.uid	: midaswww.uil
	UIL/MOTIF $<

midas.dat	: midas.ad
	COPY $< *.dat

midaswww.dat	: midaswww.ad
	COPY $< *.dat

$(NAME).exe : $(NAME).obj $(NAME).olb
	link $(NAME),$(NAME)/opt
	delete/noconf $(NAME).obj;*
	purge $(NAME).exe

exe : 
	@ $(MMS) $(MMSQUALIFIERS)/skip $(NAME).exe

test	:	all
	@ define/user decw$user_defaults sys$disk:[]
	@ midaswww := $sys$disk:[]midaswww.exe
	@ midaswww -name "midaswww"

clean :
	delete/noconfirm *.obj;*

help :
	@echo ' lib:     makes $(NAME).olb'
	@echo ' debug:   makes $(NAME)_debug.olb'
	@echo ' util:    makes utilities nt2text.exe and text2nt.exe'
	@echo ' clean:   removes all .obj files'
	@echo ' install: installs libraries and include files'

$(LIBRARY) : $(LIBMEMS)

