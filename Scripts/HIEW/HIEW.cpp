// ===================================================================== //
//                                                                       //
//       Filename:  HIEW.cpp                                             //
//    Description:  Half Intensity Emission Width Program for K line     //
//                                                                       //
//        Version:  3.2                                                  //
//        Created:  08/05/2020                                           //
//       Compiler:  g++                                                  //
//                                                                       //
//         Author:  Faiber Rosas                                         //
//          Email:  fd.rosasportilla@ugto.mx                             //
//        Company:  Dept. of Astronomy - University of Guanajuato        //
//                                                                       //
// ===================================================================== //

// ===================================================================== //
//                                                                       // 
//  Compile with:                                                        //
//                                                                       //
//  g++ -o HIEW.out HIEW.cpp                                             //
//                                                                       //
// ===================================================================== //

/* Header files */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "spline.h"

using namespace std;

/* Constants */

//#define Pi 3.14159265

/* Function prototypes */

int Count(char name[25]);
double Near_Value(double array_1[], int size, double value, double array_2[], double lim_l, double lim_u);
double Min(double array_1[], int size, double array_2[], double lim_l, double lim_u);
double Max(double array_1[], int size, double array_2[], double lim_l, double lim_u);
char * Read_Config(char config_name[20], char config[6]);

/* Main function */

int main(int argc, char** argv)
{
  if(argc<1)
    {
      printf("usage: %s <>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  
  cout<<"\n// ===================================================================== //"<<endl;
  cout<<"//      HIEW v3.2 - Half Intensity Emission Width Program for K line     //"<<endl;
  cout<<"//                                                                       //"<<endl;
  cout<<"//          Created by: Faiber Rosas - fd.rosasportilla@ugto.mx          //"<<endl;
  cout<<"//             Dept. of Astronomy - University of Guanajuato             //"<<endl;
  cout<<"// ===================================================================== //\n"<<endl;  
  
  /* In and Out files */
  
  FILE *DATA_IN_FILE; /* Data in */
  FILE *DATA_OUT_FILE; /* Data out */
  FILE *SPLINE_OUT_FILE; /* Spline out file */
  FILE *RESULTS_FILE; /* Results file */
  
  /* Initial parameters */
  
  double L_Min, L_Max, L_Lin, Range;
  int NS;
  
  /* Variables of configuration file */
  
  char CFG_FILE[]="HIEW_Config.cfg";
  char FILE[]="FILE=", OUTR[]="OUTR=", OUTD[]="OUTD=", OUTS[]="OUTS=";
  char LMIN[]="LMIN=", LMAX[]="LMAX=", LLIN[]="LLIN=", RANG[]="RANG=", NSPL[]="NSPL=";
    
  /* Read the configuration file using the Read_Config function */
  
  const char *Data_File=Read_Config(CFG_FILE, FILE); /* Name of star file */

  std::string Delete_Rows;
  Delete_Rows = "sed -i -e 1,2d ";
  Delete_Rows += Read_Config(CFG_FILE, FILE);
  
  system(Delete_Rows.c_str());
  
  DATA_IN_FILE=fopen(Read_Config(CFG_FILE, FILE),"r");
  
  L_Min=atof(Read_Config(CFG_FILE, LMIN)); /* Lambda minimum */
  L_Max=atof(Read_Config(CFG_FILE, LMAX)); /* Lambda maximum */
  L_Lin=atof(Read_Config(CFG_FILE, LLIN)); /* Lambda of central line */
  Range=atof(Read_Config(CFG_FILE, RANG)); /* Range of central line */
  NS=atof(Read_Config(CFG_FILE, NSPL)); /* Number of data generated by spline interpolation */ 

  /* Variables of STAGE 1 */
  
  int NT=Count(Read_Config(CFG_FILE, FILE)); /* The total data number is determined using the Count function */

  double * Array_L; /* Array for Lambda and Intensity */
  Array_L=new double[NT];
  
  double * Array_I;
  Array_I=new double[NT];
  
  double L, I; /* Variables Lambda and Intensity */
  double New_LMin, New_LMax; /* Variables for calculate the I_Min and I_Max */
  double Lf, If, Ll, Il, C; /* Dummy variables */
  
  cout<<"--------------------------------------------------------------------------"<<endl;
  cout<<"                           STAGE 1: Reading Data                          "<<endl;
  cout<<"--------------------------------------------------------------------------\n"<<endl;
  
  printf("Data_File=%s\n", Data_File);  /* Shows the file read */
  printf("NT=%i\n\n", NT); /* Print the total data number */
  
  /* Fill the arrays of L and I, only save the first and fourth columns */
  
  for(int i=0; i<NT; i++)     
  {
    fscanf(DATA_IN_FILE, "%lf %lf %lf", &L, &I, &C);     
    Array_L[i]=L*10; //**************************************** Por que por 10 ?
    Array_I[i]=I;
    
    /* First and last values of Ls and SF(Ls) */
    
    if(i == 0)
    {
      Lf=Array_L[i];
      If=Array_I[i];
    }
        
    if(i == (NT-1))
    {
      Ll=Array_L[i];
      Il=Array_I[i];
    }
  }
  
  fclose(DATA_IN_FILE);
  
  /* Determine the closest value to L_Min and L_Max using the Near_Value function */
  
  New_LMin=Near_Value(Array_L, NT, L_Min, Array_L, Lf, Ll);
  New_LMax=Near_Value(Array_L, NT, L_Max, Array_L, Lf, Ll);

  printf("L_Min=%0.3f | Near Value of L_Min=%f\n", L_Min, New_LMin);
  printf("L_Max=%0.3f | Near Value of L_Max=%f\n", L_Max, New_LMax);
  
  /* Output file for the spline routine */
  
  const char *Data_Out=Read_Config(CFG_FILE, OUTD); /* Name of out file for spline routine */
  DATA_OUT_FILE=fopen(Data_Out,"w");
  
  for(int i=0; i<NT; i++)
    {
      if(Array_L[i] >= New_LMin && Array_L[i] <= New_LMax)
      {
        fprintf(DATA_OUT_FILE, "%f %f\n", Array_L[i], Array_I[i]);
      }
    }
  
  fclose(DATA_OUT_FILE);
  
  int ND=Count(Read_Config(CFG_FILE, OUTD)); /* The data number is determined using the Count function */

  printf("\nND=%d\n", ND); /* Print the data number we will use */		  
  
  cout<<"\n--------------------------------------------------------------------------"<<endl;
  cout<<"                   STAGE 2: Fit of Values Using Splines                   "<<endl;
  cout<<"--------------------------------------------------------------------------\n"<<endl;
  
  double Ls, Is; /* Variables lambda and intensity for spline routine */
  std::vector<double> VL(ND), VI(ND), SVL((ND/2)), SVI((ND/2));
  
  DATA_OUT_FILE=fopen(Data_Out,"r");
  
  for(int i=0; i<ND; i++)
    {
      fscanf(DATA_OUT_FILE, "%lf %lf", &Ls, &Is);
      VL[i]=Ls;
      VI[i]=Is;
    }
  
  fclose(DATA_OUT_FILE);
  
  int NC=0;
  
  for(int i=0; i<(ND/2); i++)
    {
      if(NC == 0)
      {
        SVL[i]=(VL[0]+VL[1])/2;
        SVI[i]=(VI[0]+VI[1])/2;
      }
      
      if(NC >= ND)
      {
        SVL[i]=(VL[ND-1]+VL[ND])/2;
        SVI[i]=(VI[ND-1]+VI[ND])/2;
      }
      else
      {
        SVL[i]=(VL[NC]+VL[NC+1])/2;
        SVI[i]=(VI[NC]+VI[NC+1])/2;
      }
      
      NC=NC+2;
    }
  
  tk::spline SF; /* Spline function */
  SF.set_points(SVL,SVI);
  
  Ls=New_LMin; /* Spline interpolation from New_LMin to New_LMax */
  
  double * New_AL; /* New array for Lambda and Intensity */
  New_AL=new double[NS];
  
  double * New_AI;
  New_AI=new double[NS];
  
  const char *Spline_Out=Read_Config(CFG_FILE, OUTS); /* Name of spline out file */
  SPLINE_OUT_FILE=fopen(Spline_Out,"w");
  
  for(int i=0; i<NS; i++)
    {      
      fprintf(SPLINE_OUT_FILE, "%f %f\n", Ls, SF(Ls));
      
      New_AL[i]=Ls;
      New_AI[i]=SF(Ls);
      //printf("%f %f\n", New_AL[i], New_AI[i]);
      
      Ls=Ls+(New_LMax-New_LMin)/NS;
    }
  
  fclose(SPLINE_OUT_FILE);
  
  SPLINE_OUT_FILE=fopen(Spline_Out,"r");
  
  if(SPLINE_OUT_FILE != NULL) /* Warning */
    {
      cout<<"The fit by spline was successful!\n";  

      printf("ND=%d\n", Count(Read_Config(CFG_FILE, OUTS))); /* Print the data number generated by spline */
    }
  
  else
    {
      cout<<"\n ERROR: The fit by spline was not successful!\n";  
    }
  
  fclose(SPLINE_OUT_FILE);
  
  cout<<"\n--------------------------------------------------------------------------"<<endl;
  cout<<"                     STAGE 3: Minimum & Maximum Values                    "<<endl;
  cout<<"--------------------------------------------------------------------------\n"<<endl;
  
  double Temp_LMin, Temp_LMax;
  double I_Min[2], I_Max[2], I_Mid[2]; /* Minimum and maximum values */
  double L_IMin[2], L_IMax[2]; /* Variables for calculate the FWHM */
  
  /* Determine the minimum and maximum value of the intensity in the range of (L_Min, L_Lin-Range) and 
     (L_Lin+Range, L_Max) using the Min and Max functions */
  
  Temp_LMin=New_LMin;
  Temp_LMax=L_Lin-Range;
  
  for(int i=0; i<2; i++)     
    {
      I_Min[i]=Min(New_AI, NS, New_AL, Temp_LMin, Temp_LMax);      
      
      for(int j=0; j<NS; j++)
      {      
        if(New_AL[j] > Temp_LMin && New_AL[j] < Temp_LMax)
          {
            if(New_AI[j] == I_Min[i])
            {
              L_IMin[i]=New_AL[j];
            }	    
          }
      }

      if(i==0)
      {
        I_Max[i]=Max(New_AI, NS, New_AL, L_IMin[i], Temp_LMax);
      }

      if(i==1)
      {
        I_Max[i]=Max(New_AI, NS, New_AL, Temp_LMin, L_IMin[i]);
      }
      
      for(int j=0; j<NS; j++)
      {      
        if(New_AL[j] > Temp_LMin && New_AL[j] < Temp_LMax)
        {	      
          if(New_AI[j] == I_Max[i])
          {
            L_IMax[i]=New_AL[j];
          }
        }
      }
          
      Temp_LMin=L_Lin+Range;
      Temp_LMax=New_LMax;

      if(i == 0)
      {
        printf("Values for V peak:\n\n");
        printf("I_Min=%f | I_Max=%f | L_IMin=%f | L_IMax=%f\n", i, I_Min[i], I_Max[i], L_IMin[i], L_IMax[i]);
      }

      if(i == 1)
      {
        printf("\nValues for R peak:\n\n");
        printf("I_Min=%f | I_Max=%f | L_IMin=%f | L_IMax=%f\n", i, I_Min[i], I_Max[i], L_IMin[i], L_IMax[i]);
      }
    }
  
  cout<<"\n--------------------------------------------------------------------------"<<endl;
  cout<<"                       STAGE 4: Half Value Intensity                      "<<endl;
  cout<<"--------------------------------------------------------------------------\n"<<endl;
  
  double Temp_IMid, Temp_NV;
  double Array_FWHM[2];
  
  /* Determine the half value of the intensity using the difference of I_Min and I_Max */
  
  for(int i=0; i<2; i++)     
    {            
      I_Mid[i]=I_Min[i]+(I_Max[i]-I_Min[i])/2;
    }
  
  Temp_IMid=I_Mid[0];
  Temp_LMin=L_IMin[0];
  Temp_LMax=L_IMax[0];
  Temp_NV=Near_Value(New_AI, NS, Temp_IMid, New_AL, Temp_LMin, Temp_LMax);
  
  for(int i=0; i<2; i++)     
    {            
      for(int j=0; j<NS; j++)
      {	  	  
        if(New_AL[j] > Temp_LMin && New_AL[j] < Temp_LMax)
        {
          if(New_AI[j] == Temp_NV)
          {	      
            Array_FWHM[i]=New_AL[j];

            //printf("j=%d FWHM=%f\n", j, Array_FWHM[0]);
          }
        }
      }     

      if(i == 0)
      {
        printf("Values for V peak:\n\n");
        printf("I_Mid=%f | Near Value of I_Mid=%f | L_IMid=%f\n", I_Mid[i], Temp_NV, Array_FWHM[i]);
      }

      if(i == 1)
      {
        printf("\nValues for R peak:\n\n");
        printf("I_Mid=%f | Near Value of I_Mid=%f | L_IMid=%f\n", I_Mid[i], Temp_NV, Array_FWHM[i]);
      }
      
      Temp_IMid=I_Mid[1];
      Temp_LMin=L_IMax[1];
      Temp_LMax=L_IMin[1];
      Temp_NV=Near_Value(New_AI, NS, Temp_IMid, New_AL, Temp_LMin, Temp_LMax);
    }
  
  cout<<"\n--------------------------------------------------------------------------"<<endl;
  cout<<"                        STAGE 5: Determine the HIEW                       "<<endl;
  cout<<"--------------------------------------------------------------------------\n"<<endl;
  
  double Array_EFWHM[2];
  double FWHM, E_FWHM;
  
  /* Determine the error of FWHM using Array_FWHM[i] and the resolution of TIGRE (~20000) */
  
  for(int i=0; i<2; i++)     
    {
      Array_EFWHM[i]=abs(5.985E-3 + 1.9E-5*Array_FWHM[i])/2;
    }
  
  /* Determine the Half Intensity Emission Width */
  
  FWHM=Array_FWHM[1]-Array_FWHM[0];
  
  printf("| FWHM_LL=%f +/- %f |\n", Array_FWHM[0], Array_EFWHM[0]);
  printf("| FWHM_UL=%f +/- %f |\n\n", Array_FWHM[1], Array_EFWHM[1]);
  
  /* Determine the error of the Half Intensity Emission Width using the average */
  
  E_FWHM=abs(sqrt(pow(Array_EFWHM[0],2) + pow(Array_EFWHM[1],2)));
  
  printf("| FWHM=%f +/- %f|\n", FWHM, E_FWHM);  
  
  cout<<"\n--------------------------------------------------------------------------\n"<<endl;
  
  /* Export results to out file */
  
  RESULTS_FILE=fopen(Read_Config(CFG_FILE, OUTR),"w");
  fprintf(RESULTS_FILE, "# STAGE 1\n\n");
  fprintf(RESULTS_FILE, "Data_File=\"%s\"\n", Data_File);
  fprintf(RESULTS_FILE, "Data_Total_Number=%i\n", NT);
  fprintf(RESULTS_FILE, "Spline_Out_File=\"%s\"\n", Spline_Out);
  fprintf(RESULTS_FILE, "LMin=%f\n", New_LMin);
  fprintf(RESULTS_FILE, "LMax=%f\n\n", New_LMax);
  fprintf(RESULTS_FILE, "# STAGE 2\n\n");
  fprintf(RESULTS_FILE, "#The fit by splines was successful!\n");
  fprintf(RESULTS_FILE, "ND=%d\n\n", Count(Read_Config(CFG_FILE, OUTS)));
  fprintf(RESULTS_FILE, "# STAGE 3\n\n");
  fprintf(RESULTS_FILE, "L_IMin_1=%f\n", L_IMin[0]);
  fprintf(RESULTS_FILE, "L_IMin_2=%f\n", L_IMin[1]);
  fprintf(RESULTS_FILE, "IMin_1=%f\n", I_Min[0]);
  fprintf(RESULTS_FILE, "IMin_2=%f\n", I_Min[1]);
  fprintf(RESULTS_FILE, "L_IMax_1=%f\n", L_IMax[0]);
  fprintf(RESULTS_FILE, "L_IMax_2=%f\n", L_IMax[1]);
  fprintf(RESULTS_FILE, "IMax_1=%f\n", I_Max[0]);
  fprintf(RESULTS_FILE, "IMax_2=%f\n\n", I_Max[1]);
  fprintf(RESULTS_FILE, "# STAGE 4\n\n");
  fprintf(RESULTS_FILE, "IMid_1=%f\n", I_Mid[0]);
  fprintf(RESULTS_FILE, "IMid_2=%f\n", I_Mid[1]);
  fprintf(RESULTS_FILE, "FWHM_LL=%f\n", Array_FWHM[0]);
  fprintf(RESULTS_FILE, "FWHM_UL=%f\n\n", Array_FWHM[1]);
  fprintf(RESULTS_FILE, "ELFWHM_LL=%f\n", Array_EFWHM[0]);
  fprintf(RESULTS_FILE, "EUFWHM_LL=%f\n", Array_EFWHM[0]);
  fprintf(RESULTS_FILE, "ELFWHM_UL=%f\n", Array_EFWHM[1]);
  fprintf(RESULTS_FILE, "EUFWHM_UL=%f\n\n", Array_EFWHM[1]);
  fprintf(RESULTS_FILE, "FWHM=%f\n", FWHM);
  fprintf(RESULTS_FILE, "E_FWHM=%f\n", E_FWHM);
  fclose(RESULTS_FILE);  
  
  return EXIT_SUCCESS;   
} /* End of main */

/* Functions */

/* Counter function of the number of lines in a file */

int Count(char name[25])
{  
  FILE *File;
  int c=0;
  char temp[100]; /* If the account fails, increase the number */
  File=fopen(name,"r");
  
  while(fgets(temp,100,File) != NULL)
    {             
      if(!feof(File))
	{
	  /* Only if it's different from a control character */
	  
	  if(temp[0] != '\n' && temp[0] != '\t' && temp[0] != '\v' &&
	     temp[0] != '\b' && temp[0] != '\r' && temp[0] != '\f' &&
	     temp[0] != '\a' && temp[0] != ' ' && temp[0] != '\0')
	    {
	      c++;

	      //cout<<c<<"\n";
	      //cout<<temp;
	    }
	}
    } 
  
  fclose(File);
  
  return c; /* Return the number of lines */
} /* End of Count */

/* Read a configuration file */

char * Read_Config(char config_name[20], char config[6]) 
{
  char configuration[5];
  char temp[100];
  
  memset(temp, 0, 100); /* Initialize the char with zeros */
  
  FILE * config_file;
  config_file=fopen(config_name, "r");
  
  /* Copy the first 6 characters of each line of the configuration file */
  
  while(fgets(configuration, 6, config_file)) 
    {
      /* If are equal returns 0 */
      
      if (strcmp(configuration, config)==0)
	{
	  /* Save the value in temp */
	  fgets(temp, 100, config_file);
	}      
    }
  
  fclose(config_file);
  
  char * value = new char[strlen(temp)]; /* Char for definitive value */
  
  memset(value, 0, strlen(temp)); /* Initialize the char with zeros */
  strncpy(value, temp, strlen(temp)-1); /* Copy temp in value */
  strcat(value, "\0"); /* Add the end line \0 to value */
  
  return value; /* Return value */  
} /* End of Read_Config */

/* Determine the closest value */

double Near_Value(double array_1[], int size, double value, double array_2[], double lim_l, double lim_u)
{
  double temp;
  double near_value;
  
  for(int i=0; i<size; i++)     
  {
    if(i == 0){temp=abs(array_1[i]-value);}
    if(array_2[i] > lim_l && array_2[i] < lim_u)
    {
      if(abs(array_1[i]-value) < temp)
        {
          near_value=array_1[i];
          temp=abs(array_1[i]-value);

          //printf("n=%i array=%f value=%f near_value_1=%f temp=%f\n", i, array_1[i], value, near_value, temp);
        }
    }
  }
  
  return near_value; /* Return near_value */
} /* End of Near_Value */

/* Determine the minimum and maximum value in the range of (lim_l, lim_u) */

double Min(double array_1[], int size, double array_2[], double lim_l, double lim_u)
{
  double temp_min=1e9;
  for(int i=0; i<size; i++)     
    {      
      if(array_2[i] > lim_l && array_2[i] < lim_u)
	{	 
	  if(array_1[i] < temp_min)
	    {
	      temp_min=array_1[i];

	      //printf("n=%i L=%f I=%f I_Min=%f\n", i, array_2[i], array_1[i], temp_min);
	    }
	}
    }
  
  return temp_min; /* Return temp_min */
} /* End of Min */

double Max(double array_1[], int size, double array_2[], double lim_l, double lim_u)
{
  double temp_max=0;
  
  for(int i=0; i<size; i++)     
    {     
      if(array_2[i] > lim_l && array_2[i] < lim_u)
	{	  
	  if(array_1[i] > temp_max)
	    {
	      temp_max=array_1[i];

	      //printf("n=%i L=%f I=%f I_Max=%f\n", i, array_2[i], array_1[i], temp_max);
	    }	 
	}
    }
  
  return temp_max; /* Return temp_max */
} /* End of Max */
