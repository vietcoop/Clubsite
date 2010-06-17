<?php
// $Id$

/**
 * User invitation page callback.
 */
function w_user_invitation_callback($step = 'invite') {
  // Load includes
  ctools_include('wizard');
  ctools_include('object-cache');
  
  // Get forms info
  $form_info  = w_user_invitation_forms_info();

  $cache_name = 'wInvitation';
  $obj = ctools_object_cache_get($cache_name, $cache_name);
  if (!$obj) {
    $step = current(array_keys($form_info['order']));
    $obj  = new stdClass();
    ctools_object_cache_set($cache_name, $cache_name, $obj);
  }
  
  $form_state['cache name'] = $cache_name;
  $form_state['obj'] = $obj;

  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
  
  return $output;
}

/**
 * User invitation forms info.
 */
function w_user_invitation_forms_info() {
  return array(
    'id'   => 'w_invitation_form',
    'path' => 'invite/%step',
    'show trail'      => TRUE,
    'show back'       => TRUE,
    'show return'     => FALSE,
    'finish text'     => t('Send invitation'),
    'order' => array(
      'invite'   => t('Invite friend'),
      'preview'  => t('Preview'),
    ),
    'forms' => array(
      'invite' => array(
        'form id' => 'w_user_invitation_form',
      ),
      'preview' => array(
        'form id' => 'w_user_invitation_preview_form',
      ),
    ),
  );
}

/**
 * Invitation form.
 */
function w_user_invitation_form(&$form, &$form_state) {
  $obj = &$form_state['obj'];
  
  // Email address
  $form['emails'] = array(
    '#type'  => 'textarea',
    '#title' => t('Your friends\'s email addresses'),
    '#required' => TRUE,
    '#default_value' => !empty($obj->emails) ? implode("\n", $obj->emails) : '',
    '#description' => 'Enter one email per line.',
  );

  // Message
  $form['message'] = array(
    '#type'  => 'textarea',
    '#title' => t('Your message'),
    '#required' => TRUE,
    '#default_value' => !empty($obj->message) ? $obj->message : '',
    '#description' => t('Your message to go along with the invitations.'),
  );

  return $form;
}

function w_user_invitation_form_validate(&$form, &$form_state) {
  $v = &$form_state['values'];
  $v['emails'] = trim($v['emails'], "\n\r\t\s");

  if (!empty($v['emails'])) {
    $v['emails'] = explode("\n", $v['emails']);
    foreach (array_keys($v['emails']) as $k) {
      $mail = &$v['emails'][$k];
      $mail = trim($mail, "\r\t\s");

      if (!valid_email_address($mail)) {
        form_set_error(
          'emails', 
          t('@mail is not a valid email address', array(
            '@mail' => $mail,
          ))
        );
      }
    }
  }
  else {
    form_set_error(
      'emails', 
      t('@label can not be empty.', array(
        '@label' => $form['emails']['#title'],
      ))
    );
  }
}

function w_user_invitation_form_submit(&$form, &$form_state) {
  $obj = &$form_state['obj'];
  $cache_name = $form_state['cache name'];

  // Update values.
  $obj->emails  = $form_state['values']['emails'];
  $obj->message = $form_state['values']['message'];

  // Generate email content -- subject & body
  $obj->subject = variable_get('wUserCodeInvitationSubjectTemplate', W_USER_CODE_INVITATION_SUBJECT_TEMPLATE);
  $obj->subject = token_replace($obj->subject);
  $obj->body    = variable_get('wUserCodeInvitationBodyTemplate', W_USER_CODE_INVITATION_BODY_TEMPLATE);
  $obj->body   .= "\n\n{$obj->message}";
  $obj->body    = token_replace($obj->body);
  # $obj->body    = drupal_html_to_text($obj->body);
  $obj->body    = _filter_autop($obj->body);

  // Save cache
  ctools_object_cache_set($cache_name, $cache_name, $obj);
}

function w_user_invitation_preview_form(&$form, &$form_state) {
  $obj = &$form_state['obj'];

  $form['preview'] = array(
    '#type'  => 'fieldset',
    '#title' => $obj->subject,
    '#collapsible' => FALSE,
    '#value' => $obj->body,
  );
  
  $form['buttons']['next']['#value'] = t('Send invitation');
}

function w_user_invitation_preview_form_submit(&$form, &$form_state) {
  global $user;
  $obj = &$form_state['obj'];
  $cache_name = $form_state['cache name'];
  $v = &$form_state['values'];

  $invitorUID = $user->uid;
  $lang = user_preferred_language($user);
  $params['subject'] = $obj->subject;
  $params['body']    = $obj->body;

  foreach ($obj->emails as $friend_mail) {
    if (FALSE !== _w_user_invitation($friend_mail, $invitorUID, $lang, $params)) {
      drupal_set_message(t('Invitation sent to !mail', array('!mail' => $friend_mail)));
    }
  }

  // Clear cache
  ctools_object_cache_clear($cache_name, $cache_name);

  // Redirect user to step 1 page
  $form_state['redirect'] = 'invite';
}

function _w_user_invitation($friend_mail, $invitorUID, $lang, $params) {
  // Load include
  module_load_include('inc', 'w', 'includes/w.code');

  // Get code
  $invitation_code = w_user_code_get($friend_mail, $invitorUID);

  $params['body'] = str_replace(
    array('[link-register]', '[user-invitation-code]'),
    array(
      w_user_register_path_with_invitation_code($invitation_code),
      $invitation_code
    ), 
    $params['body']
  );

  // Send mail
  return drupal_mail('w', 'userInvitation', $friend_mail, $lang, $params);
}

function w_user_register_path_with_invitation_code($code, $absolute = TRUE) {
  return url(
    "user/register/use-code",
    array(
      'query' => "invitation-code={$code}",
      'absolute' => $absolute
    )
  );
}

/** 
 * Helper function to send invitation-mail.
 */
function _w_user_invitation_mail(&$message, $params) {
  $message['subject'] = check_plain($params['subject']);
  $message['body'][]  = drupal_html_to_text(token_replace($params['body']));
}
