#define INCL_BASE
#define INCL_NOPM
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _EXECBUNDLE
   {
   CHAR        szFailName [128];
   USHORT      fExec;
   CHAR        szArgs [128];
   CHAR        szEnvironment [255];
   RESULTCODES resc;
   CHAR        szPgmName [128];
   } EXECBUNDLE;
typedef EXECBUNDLE FAR *PEXECBUNDLE;


EXECBUNDLE      aebnd [20];
PID             pid;
SHORT           sMaxExec;

VOID            CfaStartWatchRange (SHORT sStart, SHORT sStop, LONG lDelay);
VOID            CfaStartWatchAll (SHORT sStart, SHORT sStop);
USHORT          CfaGetMakeInput (VOID);
USHORT          CfaGetExecInput (VOID);
CHAR            *CfaStrTrim (CHAR *pszLine);
VOID            main (int argc, char *argv []);

/****************************************************************************/
VOID            main (int argc, char *argv [])
{
PIDINFO         pidi;

if (argc != 3)
   {
   printf ("\n USAGE : CFEXEC <mode 1 or 2><Exec delay in Milliseconds>\n");
   return;
   }

sMaxExec = CfaGetExecInput ();
if (!sMaxExec)
   {
   return;
   }
DosGetPID (&pidi);

if (argv [1][0] == '1')
   {
   DosSetPrty (PRTYS_PROCESSTREE, PRTYC_IDLETIME, 0, pidi.pid);
   CfaStartWatchRange (0, sMaxExec - 1, atol (argv [2]));
   }
else
   {
   DosSetPrty (PRTYS_PROCESSTREE, PRTYC_REGULAR, 0, pidi.pid);
   CfaStartWatchAll (0, sMaxExec - 1);
   }
DosSetPrty (PRTYS_PROCESSTREE, PRTYC_REGULAR, 0, pidi.pid);
}
/****************************************************************************/
VOID            CfaStartWatchRange (SHORT sStart, SHORT sStop, LONG lDelay)
{
SHORT           iI;
USHORT          usRC;


for (iI = sStart; iI <= sStop; iI++)
   {
   if ((usRC = DosExecPgm (aebnd [iI].szFailName,
			   sizeof (aebnd [iI].szFailName),
			   aebnd [iI].fExec,
			   aebnd [iI].szArgs,
			   0,
			   &(aebnd [iI].resc),
			   aebnd [iI].szPgmName)) != NULL)
      {
      DosBeep (100, 400);
      fprintf (stderr, "Exited on exec %d with ecode %d", iI, usRC);
      DosExit (EXIT_PROCESS, -1);
      }
   else
      {
      DosSleep (lDelay);
      }
   }

for (iI = sStart; iI <= sStop; iI++)
   {
   if ((usRC = DosCwait (DCWA_PROCESS,
		  DCWW_WAIT,
		  &(aebnd [iI].resc),
		  &pid,
		  aebnd [iI].resc.codeTerminate)) != NULL)
      {
      DosBeep (100, 400);
      fprintf (stderr, "Exited on cwait %d with ecode %d", iI, usRC);
      DosExit (EXIT_PROCESS, -1);
      }
   else
      {
      fprintf (stderr, "Process %d done : Term %d Res %d\n", iI,
					       aebnd [iI].resc.codeTerminate,
					       aebnd [iI].resc.codeResult);
      }
   }
}
/****************************************************************************/
VOID            CfaStartWatchAll (SHORT sStart, SHORT sStop)
{
SHORT           iI;
USHORT          usRC;

for (iI = sStart; iI <= sStop; iI++)
   {
   if ((usRC = DosExecPgm (aebnd [iI].szFailName,
			   sizeof (aebnd [iI].szFailName),
			   aebnd [iI].fExec,
			   aebnd [iI].szArgs,
			   0,
			   &(aebnd [iI].resc),
			   aebnd [iI].szPgmName)) != NULL)
      {
      DosBeep (100, 400);
      fprintf (stderr, "Exited on exec %d with ecode %d", iI, usRC);
      DosExit (EXIT_PROCESS, -1);
      }
   else
      {
      if ((iI < sStop) && (stricmp (aebnd [iI].szPgmName, aebnd [iI + 1].szPgmName)))
	 {
	 if ((usRC = DosCwait (DCWA_PROCESS,
			       DCWW_WAIT,
			       &(aebnd [iI].resc),
			       &pid,
			       aebnd [iI].resc.codeTerminate)) != NULL)
	    {
	    DosBeep (100, 400);
	    fprintf (stderr, "Exited on cwait %d with ecode %d", iI, usRC);
	    DosExit (EXIT_PROCESS, -1);
	    }
	 else
	    {
	    fprintf (stderr, "Process %d done : Term %d Res %d\n", iI,
				       aebnd [iI].resc.codeTerminate,
				       aebnd [iI].resc.codeResult);
	    }

	 }
      else if (iI == sStart)
         {
         ; 
         }
      else if (!stricmp (aebnd [iI].szPgmName, aebnd [iI - 1].szPgmName))
         {
         if ((usRC = DosCwait (DCWA_PROCESS,
	                       DCWW_WAIT,
			       &(aebnd [iI - 1].resc),
                               &pid,
			       aebnd [iI - 1].resc.codeTerminate)) != NULL)
            {
            DosBeep (100, 400);
	    fprintf (stderr, "Exited on cwait %d with ecode %d", iI - 1, usRC);
            DosExit (EXIT_PROCESS, -1);
            }
         else
            {
	    fprintf (stderr, "Process %d done : Term %d Res %d\n", iI - 1,
				       aebnd [iI - 1].resc.codeTerminate,
				       aebnd [iI - 1].resc.codeResult);
	    }

         }
      }
   }

/* Wait for the last child in the range to finish */
if ((usRC = DosCwait (DCWA_PROCESS,
		      DCWW_WAIT,
		      &(aebnd [sStop].resc),
		      &pid,
		      aebnd [sStop].resc.codeTerminate)) != NULL)
   {
   DosBeep (100, 400);
   fprintf (stderr, "Exited on cwait %d with ecode %d", sStop, usRC);
   DosExit (EXIT_PROCESS, -1);
   }
else
   {
   fprintf (stderr, "Process %d done : Term %d Res %d\n", sStop,
				       aebnd [sStop].resc.codeTerminate,
				       aebnd [sStop].resc.codeResult);
   }
}
/****************************************************************************/
USHORT          CfaGetExecInput (VOID)
{
SHORT           iI;
CHAR            szDummy [128];
CHAR            *pszFirstWord;

iI = 0;
while (fgets (aebnd [iI].szArgs, sizeof (aebnd [iI].szArgs), stdin))
   {
   aebnd [iI].fExec = EXEC_ASYNCRESULT;
   CfaStrTrim (aebnd [iI].szArgs);
   aebnd [iI].szArgs [strlen (aebnd [iI].szArgs) - 1] = '\0';
   aebnd [iI].szArgs [strlen (aebnd [iI].szArgs) + 1] = '\0';
   strcpy (szDummy, aebnd [iI].szArgs);
   pszFirstWord = strtok (szDummy, " \t\n");

   fprintf (stderr, "%s\n", aebnd [iI].szArgs);
   if (!stricmp (pszFirstWord, "cl"))
      {
      aebnd [iI].szArgs [2] = '\0';
      strcpy (aebnd [iI].szPgmName, "cl.exe");
      iI++;
      }
   /*
   else if (!stricmp (pszFirstWord, "rc"))
      {
      aebnd [iI].szArgs [2] = '\0';
      strcpy (aebnd [iI].szPgmName, "rc.exe");
      iI++;
      }
   else if (!stricmp (pszFirstWord, "link"))
      {
      aebnd [iI].szArgs [4] = '\0';
      strcpy (aebnd [iI].szPgmName, "link.exe");
      iI++;
      }
   else if (!stricmp (pszFirstWord, "mapsym"))
      {
      aebnd [iI].szArgs [6] = '\0';
      strcpy (aebnd [iI].szPgmName, "mapsym.exe");
      iI++;
      }
   else if (!stricmp (pszFirstWord, "implib"))
      {
      aebnd [iI].szArgs [6] = '\0';
      strcpy (aebnd [iI].szPgmName, "implib.exe");
      iI++;
      }
   else if (!stricmp (pszFirstWord, "bind"))
      {
      aebnd [iI].szArgs [4] = '\0';
      strcpy (aebnd [iI].szPgmName, "bind.exe");
      iI++;
      }
   else if (!stricmp (pszFirstWord, "echo"))
      {
      aebnd [iI].szArgs [4] = '\0';
      strcpy (aebnd [iI].szPgmName, "echo");
      iI++;
      }
   */
   }
return (iI);
}
/****************************************************************************/
CHAR            *CfaStrTrim (CHAR *pszLine)
{
SHORT           sCount;
size_t          sizetStrLength;

sizetStrLength = strlen (pszLine);
sCount = 0;
while (pszLine [sCount] == ' ')
   {
   sCount += 1;
   }
memmove (pszLine, pszLine + sCount, sizetStrLength - sCount);
pszLine [sizetStrLength - sCount] = '\0';
sizetStrLength = strlen (pszLine);
sCount = sizetStrLength - 1;
while (pszLine [sCount] == ' ')
   {
   sCount -= 1;
   }
pszLine [sCount + 1] = '\0';
return (pszLine);
}
/****************************************************************************/
