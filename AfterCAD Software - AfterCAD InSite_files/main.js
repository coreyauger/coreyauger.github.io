// JavaScript Document

// Arrow switch for main navigation

arrowon = new Image();
arrowon.src = '../_images/nav-arrow-on.gif';
arrowoff = new Image();
arrowoff.src = '../_images/nav-arrow.gif';

function blurry(obj) {
	if (!document.layers) obj.blur();
}

function toggleNav(n) {
	arr = document.images['navarr'+n];
	men = document.getElementById('nav'+n).style;
	if (arr.src==arrowon.src) {
		arr.src = arrowoff.src;
		men.display = 'none';
	} else {
		arr.src = arrowon.src;
		men.display = 'block';
	}
}

// Arrow switch for sub navigation

arrowonsub = new Image();
arrowonsub.src = '../_images/nav-arrow-sub-on.gif';
arrowoffsub = new Image();
arrowoffsub.src = '../_images/nav-arrow-sub.gif';

function blurry(obj) {
	if (!document.layers) obj.blur();
}

function toggleNavSub(n) {
	arr = document.images['navarrsub'+n];
	men = document.getElementById('navsub'+n).style;
	if (arr.src==arrowonsub.src) {
		arr.src = arrowoffsub.src;
		men.display = 'none';
	} else {
		arr.src = arrowonsub.src;
		men.display = 'block';
	}
}

// Mouseover and preload

function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}


// solutions page

function switch1(div) {
if (document.getElementById('one')) {
var option=['one','two','three','four','five','six'];
for(var i=0; i<option.length; i++)
{ obj=document.getElementById(option[i]);
obj.style.display=(option[i]==div)? "block" : "none"; }
}
}






