
var maxSize=100;

$(document).ready(function () { 
  $('.results img').load(function () {
    var height =$(this).height();
    var width = $(this).width();
    if (height > width)
    {
      if ( height > maxSize) 
      {
	$(this).height(maxSize);
	$(this).width( maxSize*width/height );
      }
      
    } else
    {
      if (width > maxSize) 
      {
	$(this).width(maxSize);
	$(this).height(maxSize*height/width);
	$(this).css('margin-top',  (maxSize-$(this).height())/2+'px');
      }
	
    }
  });

    $('.results tr').each(function () {
      $(this).find('div.metadata ul li:nth-child(odd)').addClass('alternate');   
    });
    
    $('.facet-list ul ul').each(function () { 
      $(this).find('li:nth-child(odd)').addClass('alternate');
    });
  
});
