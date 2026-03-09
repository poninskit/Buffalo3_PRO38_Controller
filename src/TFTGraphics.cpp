// #include <TFTGraphics.h>

// //------------------------------------------------------------------------------
// TFTGraphics::TFTGraphics(byte model, int RS, int WR, int CS, int RST, int SER)
//             :UTFT(model, RS, WR, CS, RST, SER)
// {
//   InitLCD(LANDSCAPE);
//   clrScr();
// }


// //------------------------------------------------------------------------------
// void TFTGraphics::printButtons(PAGE page)
// {
//   if( page == MAIN_MENU ){
//     drawBitmap(30,180, 74, 74, vdown);
//     drawBitmap(150,180, 74, 74, vup);
//   }

//   drawBitmap(390,20, 74, 74, set);
  
//   //drawBitmap(390,20, 74, 74, power);
//   //drawBitmap(285,20, 74, 74, set);
//   //drawBitmap(255,150, 42, 34, mute);
// }

// //------------------------------------------------------------------------------
// void TFTGraphics::printVolume( uint8_t volume )
// {

//   int x = 294;
//   int y = 134;
//   int n = 80; //font space

//   setFont( arial_numbers_80x120 );
//   setColor(10, 130,40);


//    if ( volume  < 10 ){
//      print( (char *)"0", x, y );
//      print( String( volume ), x + n, y);
//    }else{
//      print( String( volume ), x, y);
//    }


// }

// //------------------------------------------------------------------------------
// void TFTGraphics::printChannel( DAC_INPUT input_id )
// {

//   switch ( input_id ){
//   case USB:
//     drawBitmap(25,20, 204, 74, usb);
//     break;
//   case SPDIF:
//     drawBitmap(25,20, 204, 74, spdif);
//     break;
//   case OPT1:
//     drawBitmap(25,20, 204, 74, opt1);
//     break;
//   case OPT2:
//     drawBitmap(25,20, 204, 74, opt2);
//     break;
//   default:
//     break;
//   }

// }

// //index = -1 prints all settings, index == 1 optimize printing - only one button to refresh
// //------------------------------------------------------------------------------
// void TFTGraphics::printSettings( Settings *settings, int8_t index )
// {

//   int x = 30;
//   int y = 22; 
//   int h = 50;
//   int l = 320;
//   int jump = 60; 

//   setFont(arial_bold);
//   setBackColor( 210, 210, 210 );
  
//   for (int i = 0 ; i < 4 ; i++ ){

//     //Optimize drawing, draw only what changed
//     if ( index != -1 ){ // -1 = print all, otherwise print one selected
//       //print only given index, skip otherwise
//       if( i != index ) { 
//         y = y + jump;  // move to next position 
//         continue; //skip drawing this button
//       }
//     }

//     if ( index == -1 ){   //dont reprint whole button, value selected only
//       //Frame
//       setColor( 210, 210, 210 );
//       fillRoundRect ( x, y, x + l, y + h );
//       setColor( 70, 100, 70 );
//       drawRoundRect ( x, y, x + l + 1, y + h + 1 );
//       drawRoundRect ( x, y, x + l + 2, y + h + 2 );
//       //Name 
//       setColor( VGA_BLACK );
//       print( settings[i].sett_name, x + 10, y + 8 );
//     }
//     //Value
//     setColor(10, 130,40);
//     print( "                  ", x + 10, y + 28 );
//     print( settings[i].value_string, x + 10, y + 28 );
//     //next    
//     y = y + jump;
//   }

//   setBackColor( VGA_BLACK );
   
// }

// //------------------------------------------------------------------------------
// void TFTGraphics::printInfoText( const char* text, LINE line )
// {

//   setFont(arial_bold);
//   setColor(200, 200, 200);
//   int x = 35;
//   int y = (line == FIRST_LINE)? 105 : 135;

//   print(text, x, y);
// }

// //------------------------------------------------------------------------------
// void TFTGraphics::clearInfoText( LINE line )
// {
//   const char* strg = (line == FIRST_LINE)? "                    " : "               "; //20 or 15 char
//   printInfoText(strg, line);
// }
// //------------------------------------------------------------------------------
