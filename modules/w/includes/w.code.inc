<?php
// $Id$

/**
 * @file Functions to manage userCode.
 */

define('W_COOKIE_USER_CODE', 'wUserCode');

/**
 * Check is the userCode already used.
 */
function w_user_code_is_duplicated($code) {
  $counter = db_result(db_query(
    "SELECT COUNT(*) FROM {w_user_code} WHERE code LIKE '%s'",
    $code
  ));

  return $counter > 0;
}

function w_user_code_is_valid_format($code) {
  $sub = "[0-9A-F]{4}";
  return preg_match("/^{$sub}-{$sub}-{$sub}-{$sub}$/", $code);
}

function w_user_code_in_queue($code) {
  return (bool)db_result(db_query(
    "SELECT COUNT(*) FROM {w_user_code} WHERE code LIKE '%s' AND uid = 0",
    $code
  ));
}

function w_user_code_used($code) {
  return (bool)db_result(db_query(
    "SELECT COUNT(*) FROM {w_user_code} WHERE code LIKE '%s' AND uid <> 0",
    $code
  ));
}

function w_user_code_validate($code) {
  if (!w_user_code_is_valid_format($code)) {
    return t('Invalid code format.');
  }

  if (w_user_code_used($code)) {
    return t('Code is already used.');
  }

  if (!w_user_code_in_queue($code)) {
    return t('Code not found.');
  }

  return TRUE;
}

/**
 * Get userCode by userID.
 */
function w_user_code($uid) {
  return db_result(db_query("SELECT code FROM {w_user_code} WHERE uid = %d", $uid));
}

function w_user_code_remove($uid) {
  db_query("DELETE FROM {w_user_code} WHERE uid = %d", $uid);
}

/**
 * Get user code for a friend.
 * Create new code if it is not existing.
 * @return string Saved user code.
 */
function w_user_code_get($friend_mail, $invitor_uid = 0, $code_length = 16) {
  // Find existing user
  $friendUID = db_result(db_query(
    "SELECT uid FROM {users} WHERE mail = '%s'",
    $friend_mail
  ));

  // Friend is already.
  if ($friendUID) {
    $code = db_result(db_query("SELECT code FROM {w_user_code} WHERE uid = %d", $friendUID));
  }
  // Trying to find generated code for the email address.
  else {
    $code = db_result(db_query(
      "SELECT wuc.code"
        . " FROM {w_user_code} wuc"
        . " LEFT JOIN {w_user_code_extra} wuce ON wuc.code = wuce.code"
        . " WHERE wuce.friend_mail = '%s'",
      $friend_mail
    ));
  }
  
  // Code is not existing, create a new one.
  if (empty($code)) {
    $record = new stdClass;
    $record->code = w_user_code_generate($code_length);
    $record->uid  = (int)$friendUID;
    $record->extra = (object)array(
      'invitor_uid' => $invitor_uid,
      'created' => time(),
      'friend_mail' => $friend_mail
    );

    if (FALSE !== w_user_code_save($record)) {
      $code = $record->code;
    }
  }

  return !empty($code) ? $code : FALSE;
}

/**
 * Generate the random code look like: XXXX-XXXX-XXXX-XXXX.
 */
function w_user_code_generate($length = 16) {
  $chars = "0123456789ABCDEF";
  $size  = strlen($chars);
  $code  = '';
  $counter = 1;
  while ($counter <= $length) {
    $code .= substr($chars, rand(0, $size - 1), 1);
    if ($counter < $length && $counter % 4 === 0) {
      $code .= '-';
    }

    $counter++;
  }

  if (w_user_code_is_duplicated($code)) {
    return w_user_code_generate($length);
  }

  return $code;
}

/**
 * Save a user-code record to database.
 * @TODO: Code update.
 */
function w_user_code_save($record) {
  // Write extra informations first
  if (!empty($record->extra)) {
    $extra = $record->extra;
    $extra->code = $record->code;
    drupal_write_record('w_user_code_extra', $record->extra);
  }

  return drupal_write_record('w_user_code', $record);
}

function w_user_code_use($code, $userID) {
  db_query(
    "UPDATE {w_user_code} SET uid = %d WHERE code = '%s'",
    $userID, $code
  );

  db_query(
    "UPDATE {w_user_code_extra} SET friend_mail = '' WHERE code = '%s'",
    $code
  );
}
