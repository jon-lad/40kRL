--Effects for pickable items

--List of different effects 
--health effect
if EffectType == 1 then
  
  function healthEffect(targetedActor, amount)
    if amount > 0 then
      targetedActor.destructible:heal(amount);
    end
    if amount < 0 then
      targetedActor.destructible:takeDamage(-amount)
  end
end

-- change ai effect
  if EffectType == 2 then
    function changeAiEffect(targetedActor)
      oldAi = targetedActor.ai;
  
    end
  end
end
