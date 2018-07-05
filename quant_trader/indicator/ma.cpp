#include "ma.h"

MA::MA(int MAPeriod, int MAShift, ENUM_MA_METHOD MAMethod, ENUM_APPLIED_PRICE AppliedPrice, QObject *parent) :
    MQL5IndicatorOnSingleDataBuffer(1, AppliedPrice, parent),
    InpMAPeriod(MAPeriod),
    InpMAShift(MAShift),
    InpMAMethod(MAMethod)
{
}

//+------------------------------------------------------------------+
//|   simple moving average                                          |
//+------------------------------------------------------------------+
void MA::CalculateSimpleMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price)
  {
   int i,limit;
//--- first calculation or number of bars was changed
   if(prev_calculated==0)// first calculation
     {
      limit=InpMAPeriod+begin;
      //--- set empty value for first limit bars
      for(i=0;i<limit-1;i++) ExtLineBuffer[i]=0.0;
      //--- calculate first visible value
      double firstValue=0;
      for(i=begin;i<limit;i++)
         firstValue+=price[i];
      firstValue/=InpMAPeriod;
      ExtLineBuffer[limit-1]=firstValue;
     }
   else limit=prev_calculated-1;
//--- main loop
   for(i=limit;i<rates_total && !IsStopped();i++)
      ExtLineBuffer[i]=ExtLineBuffer[i-1]+(price[i]-price[i-InpMAPeriod])/InpMAPeriod;
//---
  }
//+------------------------------------------------------------------+
//|  exponential moving average                                      |
//+------------------------------------------------------------------+
void MA::CalculateEMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price)
  {
   int    i,limit;
   double SmoothFactor=2.0/(1.0+InpMAPeriod);
//--- first calculation or number of bars was changed
   if(prev_calculated==0)
     {
      limit=InpMAPeriod+begin;
      ExtLineBuffer[begin]=price[begin];
      for(i=begin+1;i<limit;i++)
         ExtLineBuffer[i]=price[i]*SmoothFactor+ExtLineBuffer[i-1]*(1.0-SmoothFactor);
     }
   else limit=prev_calculated-1;
//--- main loop
   for(i=limit;i<rates_total && !IsStopped();i++)
      ExtLineBuffer[i]=price[i]*SmoothFactor+ExtLineBuffer[i-1]*(1.0-SmoothFactor);
//---
  }
//+------------------------------------------------------------------+
//|  linear weighted moving average                                  |
//+------------------------------------------------------------------+
void MA::CalculateLWMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price)
  {
   int        i,limit;
   static int weightsum;
   double     sum;
//--- first calculation or number of bars was changed
   if(prev_calculated==0)
     {
      weightsum=0;
      limit=InpMAPeriod+begin;
      //--- set empty value for first limit bars
      for(i=0;i<limit;i++) ExtLineBuffer[i]=0.0;
      //--- calculate first visible value
      double firstValue=0;
      for(i=begin;i<limit;i++)
        {
         int k=i-begin+1;
         weightsum+=k;
         firstValue+=k*price[i];
        }
      firstValue/=(double)weightsum;
      ExtLineBuffer[limit-1]=firstValue;
     }
   else limit=prev_calculated-1;
//--- main loop
   for(i=limit;i<rates_total && !IsStopped();i++)
     {
      sum=0;
      for(int j=0;j<InpMAPeriod;j++) sum+=(InpMAPeriod-j)*price[i-j];
      ExtLineBuffer[i]=sum/weightsum;
     }
//---
  }
//+------------------------------------------------------------------+
//|  smoothed moving average                                         |
//+------------------------------------------------------------------+
void MA::CalculateSmoothedMA(int rates_total,int prev_calculated,int begin,const _TimeSeries<double> &price)
  {
   int i,limit;
//--- first calculation or number of bars was changed
   if(prev_calculated==0)
     {
      limit=InpMAPeriod+begin;
      //--- set empty value for first limit bars
      for(i=0;i<limit-1;i++) ExtLineBuffer[i]=0.0;
      //--- calculate first visible value
      double firstValue=0;
      for(i=begin;i<limit;i++)
         firstValue+=price[i];
      firstValue/=InpMAPeriod;
      ExtLineBuffer[limit-1]=firstValue;
     }
   else limit=prev_calculated-1;
//--- main loop
   for(i=limit;i<rates_total && !IsStopped();i++)
      ExtLineBuffer[i]=(ExtLineBuffer[i-1]*(InpMAPeriod-1)+price[i])/InpMAPeriod;
//---
  }
//+------------------------------------------------------------------+
//| Custom indicator initialization function                         |
//+------------------------------------------------------------------+
void MA::OnInit()
  {
//--- indicator buffers mapping
   SetIndexBuffer(0,ExtLineBuffer,INDICATOR_DATA);
//--- set accuracy
   IndicatorSetInteger(INDICATOR_DIGITS,_Digits+1);
//--- sets first bar from what index will be drawn
   PlotIndexSetInteger(0,PLOT_DRAW_BEGIN,InpMAPeriod);
//---- line shifts when drawing
   PlotIndexSetInteger(0,PLOT_SHIFT,InpMAShift);
//--- name for DataWindow
   string short_name="unknown ma";
   switch(InpMAMethod)
     {
      case MODE_EMA :  short_name="EMA";  break;
      case MODE_LWMA : short_name="LWMA"; break;
      case MODE_SMA :  short_name="SMA";  break;
      case MODE_SMMA : short_name="SMMA"; break;
     }
   IndicatorSetString(INDICATOR_SHORTNAME,short_name+"("+string(InpMAPeriod)+")");
//---- sets drawing line empty value--
   PlotIndexSetDouble(0,PLOT_EMPTY_VALUE,0.0);
//---- initialization done
  }
//+------------------------------------------------------------------+
//|  Moving Average                                                  |
//+------------------------------------------------------------------+
int MA::OnCalculate (const int rates_total,                     // size of the price[] array
                     const int prev_calculated,                 // bars handled on a previous call
                     const int begin,                           // where the significant data start from
                     const _TimeSeries<double>& price      // array to calculate
                     )
  {
//--- check for bars count
   if(rates_total<InpMAPeriod+1+begin)
      return(0);// not enough bars for calculation
//--- first calculation or number of bars was changed
   if(prev_calculated==0)
      ArrayInitialize(ExtLineBuffer,0);
//--- sets first bar from what index will be draw
   PlotIndexSetInteger(0,PLOT_DRAW_BEGIN,InpMAPeriod-1+begin);

//--- calculation
   switch(InpMAMethod)
     {
      case MODE_EMA:  CalculateEMA(rates_total,prev_calculated,begin,price);        break;
      case MODE_LWMA: CalculateLWMA(rates_total,prev_calculated,begin,price);       break;
      case MODE_SMMA: CalculateSmoothedMA(rates_total,prev_calculated,begin,price); break;
      case MODE_SMA:  CalculateSimpleMA(rates_total,prev_calculated,begin,price);   break;
     }
//--- return value of prev_calculated for next call
   return(rates_total);
  }
//+------------------------------------------------------------------+
