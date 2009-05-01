// $Id$

$(function() {
  // Block administration page.
  $('#region-manager-blocks-form tr.module input').change(function() {
    var classes = $(this).parents('td').getClassNames();
    // build our selector from the classes
    var selector = 'td.' + classes.join('.');
    // disable the checkboxes depending on whether the checkbox was checked or not.
    $(selector + ' input').not('[id*=all-blocks]').attr('disabled', $(this).is(':checked'));
  });
});

$.fn.getClassNames = function() {
  if (name = this.attr("class")) {
    return name.split(" ");
  }
  else {
    return [];
  }
}