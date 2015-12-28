// JavaScript Document

// Pop-up Window Script
// <a href="javascript:popUpWindow('your-page.htm', '10', '10', '640', '500')">
// url of pop-up window
// width from upper left of screen
// height from upper left of screen
// width of window
// height of window

var popUpWin=0;
function popUpWindow(URLStr, left, top, width, height)
{
  if(popUpWin)
  {
    if(!popUpWin.closed) popUpWin.close();
  }
  popUpWin = open(URLStr, 'popUpWin', 'toolbar=no,location=no,directories=no,status=no,menub ar=no,scrollbar=yes,resizable=yes,copyhistory=yes,width='+width+',height='+height+',left='+left+', top='+top+',screenX='+left+',screenY='+top+'');
}