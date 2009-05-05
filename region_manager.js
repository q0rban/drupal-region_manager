// $Id$

$(function() {
  // Block administration page.
  var selects = $('#region-manager-blocks-form tr.module select');
  selects.change(function() {
    var module = $(this).parents('tr').getClassNames().slice(0, 1);
    showHideRows(module);

    var classes = $(this).parents('td').getClassNames();
    // build our selector from the classes
    var selector = 'td.' + classes.join('.');
    blockCheckboxes(this.value, selector);

  });
  selects.change();
});

showHideRows = function(module) {
  var show = 0;
  $('#region-manager-blocks-form tr.module.' + module + ' select').each(function(i) {
    if (this.value == 2) {
      show = 1;
    }
  });
  if (show) {
    $('#region-manager-blocks-form tr[class^=' + module + ']').not('tr.' + module).show();
  }
  else {
    $('#region-manager-blocks-form tr[class^=' + module + ']').not('tr.' + module).hide();
  }
}

blockCheckboxes = function(value, selector) {
  switch(value) {
    case '0':
      $(selector + ' input').not('[id*=all-blocks]').attr('checked', 1);
      $(selector + ' input').not('[id*=all-blocks]').attr('disabled', 1);
      break;

    case '1':
      $(selector + ' input').not('[id*=all-blocks]').attr('checked', 0);
      $(selector + ' input').not('[id*=all-blocks]').attr('disabled', 1);
      break;

    case '2':
      $(selector + ' input').not('[id*=all-blocks]').attr('disabled', 0);
      break;
  }
}

$.fn.getClassNames = function() {
  if (name = this.attr("class")) {
    return name.split(" ");
  }
  else {
    return [];
  }
}